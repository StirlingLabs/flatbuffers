#nullable enable
using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Channels;
using System.Threading.Tasks;
using JetBrains.Annotations;
using StirlingLabs.MsQuic;
using StirlingLabs.Utilities;
using StirlingLabs.Utilities.Collections;

namespace BigBuffers.Xpc.Quic;

[PublicAPI]
public abstract partial class QuicRpcServiceServerBase
{
  internal ConcurrentDictionary<QuicRpcServiceServerContext, _> _Clients { get; } = new();

  public ICollection<QuicRpcServiceServerContext> Clients => _Clients.Keys;

  protected readonly TextWriter? Logger;
  public QuicListener Listener { get; }

  protected SizedUtf8String Utf8ServiceId { get; }

  private static readonly Regex RxSplitPascalCase = new(@"(?<=[a-z])([A-Z])", RegexOptions.Compiled | RegexOptions.CultureInvariant);

  private static readonly SizedUtf8String
    Utf8ErrorBadRequest = SizedUtf8String.Create("Bad Request"),
    Utf8ErrorUnauthorized = SizedUtf8String.Create("Unauthorized"),
    Utf8ErrorNotFound = SizedUtf8String.Create("Not Found"),
    Utf8ErrorRequestTimeout = SizedUtf8String.Create("Request Timeout"),
    Utf8ErrorGone = SizedUtf8String.Create("Gone"),
    Utf8ErrorNotImplemented = SizedUtf8String.Create("Not Implemented"),
    Utf8ErrorInternalServerError = SizedUtf8String.Create("Internal Server Error");

  [Discardable]
  protected internal static double TimeStamp => SharedCounters.GetTimeSinceStarted().TotalSeconds;

  protected QuicRpcServiceServerBase(string serviceId, QuicListener listener, TextWriter? logger = null)
  {
    Logger = logger;
    Utf8ServiceId = serviceId ?? throw new ArgumentNullException(nameof(serviceId));
    Listener = listener ?? throw new ArgumentNullException(nameof(listener));
    Listener.ClientConnected += ClientConnectedHandler;
  }

  private void ClientConnectedHandler(QuicListener _, QuicServerConnection connection)
  {
    var ctx = new QuicRpcServiceServerContext(this, connection);
    var success = TryAddClient(ctx);
    Debug.Assert(success);
  }

  private bool TryAddClient(QuicRpcServiceServerContext client)
    => _Clients.TryAdd(client, default);

  protected TMethodEnum ParseRequest<TMethodEnum>(RequestMessage req, out ByteBuffer bb)
    where TMethodEnum : struct, Enum
  {
    var method = SelectMethod<TMethodEnum>(req);
    var body = req.Body;
    if (body.Length > 0)
      bb = new(body);
    else
      bb = new(0);
    return method;
  }

  protected TMethodEnum SelectMethod<TMethodEnum>(in RequestMessage req) where TMethodEnum : struct, Enum
  {
    var serviceName = req.ServiceId;

    if (serviceName != Utf8ServiceId)
      return default;

    var procName = req.RpcMethod;

    var methods = (TMethodEnum[])typeof(TMethodEnum).GetEnumValues();

    foreach (var method in methods)
    {
      if (Unsafe.As<TMethodEnum, nint>(ref Unsafe.AsRef(method)) == 0)
        continue;

      if (procName == ResolveMethodSignature(method))
        return method;
    }

    return default;
  }


#if NETSTANDARD2_0
  [MethodImpl(MethodImplOptions.AggressiveInlining)]
  private static unsafe ReadOnlySpan<T> CreateReadOnlySpan<T>(ref T item, int length)
    => new(Unsafe.AsPointer(ref item), 1);
#endif

  [MethodImpl(MethodImplOptions.AggressiveInlining)]
  private void WriteErrorCode(IMessage response, long errCode)
  {
#if NETSTANDARD2_0
    Append(response, MemoryMarshal.AsBytes(CreateReadOnlySpan(ref errCode, 1)));
#else
    Append(response, MemoryMarshal.AsBytes(MemoryMarshal.CreateReadOnlySpan(ref errCode, 1)));
#endif
  }

  [MethodImpl(MethodImplOptions.AggressiveInlining)]
  private void WriteNullByte(IMessage response)
  {
    byte nullByte = 0;
#if NETSTANDARD2_0
    Append(response, MemoryMarshal.AsBytes(CreateReadOnlySpan(ref nullByte, 1)));
#else
    Append(response, MemoryMarshal.AsBytes(MemoryMarshal.CreateReadOnlySpan(ref nullByte, 1)));
#endif
  }

  protected async Task RunAsync<TMethodEnum>(CancellationToken ct = default) where TMethodEnum : struct, Enum
  {
    await _messageConsumerReady.Task;
    Debug.Assert(MessageConsumer is not null);
    do
    {
      if (ct.IsCancellationRequested) break;
      var mc = MessageConsumer;
      Debug.Assert(mc is not null);
      await foreach (var msg in mc)
      {
        if (ct.IsCancellationRequested) break;
        await Dispatch<TMethodEnum>(msg, ct);
        if (ct.IsCancellationRequested) break;
      }
    } while (ct.IsCancellationRequested);
  }

  protected async Task Dispatch<TMethodEnum>(IMessage sourceMsg, CancellationToken cancellationToken)
    where TMethodEnum : struct, Enum
  {
    var ctx = (QuicRpcServiceServerContext)sourceMsg.Context!;

    async Task SendUnaryReply(IMessage? reply)
    {
      reply ??= OnUnhandledMessage(sourceMsg, cancellationToken);

      if (reply is null)
        await NotFoundReply(ctx, sourceMsg.Id).SendAsync();
    }

    if (sourceMsg is RequestMessage req)
    {
      var msgId = req.Id;
      var msgType = req.Type;
      var method = ParseRequest<TMethodEnum>(req, out var bb);

      var methodType = ResolveMethodType(method);

      async Task Unary()
      {
        Logger?.WriteLine(
          $"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: {method} dispatching to unary {method} implementation");
        var dispatch = DispatchUnary(method, msgId, bb, cancellationToken);

        var reply = await HandleExceptions(dispatch, ctx, msgId, cancellationToken);

        await SendUnaryReply(reply);
      }

      async Task ClientStreaming()
      {
        Logger?.WriteLine(
          $"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: dispatched client streaming request is {method}");

        var isNewStream = false;
        var c = ctx.ClientMsgStreams.GetOrAdd(msgId, _ => {
          isNewStream = true;
          var newStream = new AsyncProducerConsumerCollection<IMessage>();
          OnNewClientStream(ctx, msgId, newStream);
          return newStream;
        });

        Logger?.WriteLine(
          $"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: dispatched client streaming request is {method} {msgType} and {(isNewStream ? "is" : "isn't")} a new stream");

        if ((msgType & MessageType.Control) == 0)
        {
          var added = c.TryAdd(sourceMsg);
          Debug.Assert(added);

          if (!added)
          {
            Logger?.WriteLine(
              $"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: message was not added as the stream was already closed!");
          }
        }
        else
        {
          // TODO: handle control messages
        }

        if ((msgType & MessageType.Final) != 0)
        {
          Logger?.WriteLine(
            $"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: message was final so the stream is being closed");

          c.CompleteAdding();
        }

        if (!isNewStream)
        {
          return;
        }

        // long await
        Logger?.WriteLine(
          $"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: {method} dispatching to client streaming {method} implementation");
        var dispatch = DispatchClientStreaming(method, msgId, c, cancellationToken);

        var reply = await HandleExceptions(dispatch, ctx, msgId, cancellationToken);

        await SendUnaryReply(reply);

        ctx.ClientMsgStreams.TryRemove(msgId, out var _);
        Logger?.WriteLine(
          $"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: marking messages complete due to implementation completed");
        c.CompleteAdding();
        c.Clear();

      }

      async Task ServerStreaming()
      {
        Logger?.WriteLine(
          $"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: dispatched server streaming request is {method}");

        // NOTE: does not wait on the task to complete
        await Task.Factory.StartNew(async () => {
          Logger?.WriteLine($"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: {method} started streaming");
          var t = DispatchServerStreaming(method, msgId, bb, cancellationToken);
          await t;
          Logger?.WriteLine($"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: {method} finished streaming");
        }, cancellationToken, TaskCreationOptions.LongRunning, TaskScheduler.Current);
      }

      async Task Streaming()
      {

        var isNewStream = false;
        var c = ctx.ClientMsgStreams.GetOrAdd(msgId, _ => {
          isNewStream = true;
          var newStream = new AsyncProducerConsumerCollection<IMessage>();
          OnNewClientStream(ctx, msgId, newStream);
          return newStream;
        });

        Logger?.WriteLine(
          $"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: dispatched bidirectional streaming request is {method} {msgType} and {(isNewStream ? "is" : "isn't")} a new stream");

        if ((msgType & MessageType.Control) == 0)
        {
          var added = c.TryAdd(sourceMsg);
          //Debug.Assert(added);

          if (!added)
          {
            Logger?.WriteLine(
              $"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: message was not added as the stream was already closed!");
          }
        }
        else
        {
          // TODO: handle control messages
        }

        if ((msgType & MessageType.Final) != 0)
        {
          Logger?.WriteLine(
            $"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: message was final so the stream is being closed");

          c.CompleteAdding();
        }

        if (!isNewStream)
        {
          return;
        }

        // NOTE: does not wait on the task to complete
        await Task.Factory.StartNew(async () => {
          Logger?.WriteLine($"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: {method} started streaming");
          var t = DispatchStreaming(method, msgId, c, cancellationToken);
          await t;
          Logger?.WriteLine($"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: {method} finished streaming");
          ctx.ClientMsgStreams.TryRemove(msgId, out var _);
          Logger?.WriteLine($"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: marking stream complete");
          c.CompleteAdding();
          Debug.Assert(c.IsAddingCompleted);
          c.Clear();
          Debug.Assert(c.IsCompleted);
          Logger?.WriteLine($"[{TimeStamp:F3}] {GetType().Name} #{msgId} T{Task.CurrentId}: {method} cleaned up");
        }, cancellationToken, TaskCreationOptions.LongRunning, TaskScheduler.Current);
      }

      if ((msgType & MessageType.Continuation) != 0)
      {
        if (ctx.ClientMsgStreams.TryGetValue(msgId, out var c))
        {
          if (!c.TryAdd(sourceMsg))
          {
            // collection is already complete or completing
            // either the method has already ended or aborted
            Debug.Assert(c.IsAddingCompleted);
            GoneReply(ctx, msgId);
            return;
          }

          if ((msgType & MessageType.Final) != 0)
            c.CompleteAdding();
          return;
        }

        // the collection was not found, so clean up has
        // already been run or it was not created       
        NotFoundReply(ctx, msgId);
        return;
      }

      switch (methodType)
      {
        // @formatter:off
        case RpcMethodType.Unary: await Unary(); break;
        case RpcMethodType.ClientStreaming: await ClientStreaming(); break;
        case RpcMethodType.ServerStreaming: await ServerStreaming(); break;
        case RpcMethodType.BidirectionalStreaming: await Streaming(); break;
        // @formatter:on
        default:
          throw new NotImplementedException();
      }
    }
  }

  private FairAsyncConsumerIMux<IMessage>? _messageConsumer;

  private FairAsyncConsumerIMux<IMessage>? MessageConsumer
  {
    get => Interlocked.CompareExchange(ref _messageConsumer, null, null);
    set => Interlocked.Exchange(ref _messageConsumer, value);
  }

  private TaskCompletionSource<bool> _messageConsumerReady = new();

  private void OnNewClientStream(QuicRpcServiceServerContext ctx, long msgId, AsyncProducerConsumerCollection<IMessage> newQueue)
  {
    var mc = MessageConsumer;
    if (mc is null)
    {
      _messageConsumerReady.SetResult(true);
      MessageConsumer = new(newQueue);
      return;
    }
    mc.WithLock(c => {
      var index = c.Index;

      IEnumerable<AsyncProducerConsumerCollection<IMessage>> GetMessageQueues()
      {
        var yieldIndex = -1;
        // order clients by their outbound stream ids, which should always be increasing monotonically 
        foreach (var client in Clients.OrderBy(x => x.ControlStreamOutbound!.Id))
        {
          ++yieldIndex;

          // order the queues by their message id, which should be increasing monotonically
          var orderedQueues = client.ClientMsgStreams
            .OrderBy(kv => kv.Key)
            .Select(kv => kv.Value);

          foreach (var queue in orderedQueues)
            yield return queue;

          if (client != ctx)
            continue;

          // to keep a consistent order, if yieldIndex is less than or equal to index (the current, already read item),
          if (yieldIndex <= index)
            // the new index should be +1 so the "next item" is not repeated twice
            index += 1;

          // the new message id will always be higher than previous message queues yielded before this
          yield return newQueue;
        }
      }

      var queues = GetMessageQueues();
      var newConsumer = new FairAsyncConsumerIMux<IMessage>(queues);
      newConsumer.WithLock(nc => {
        MessageConsumer = newConsumer;
        nc.Index = index;
      });
      c.Dispose();
    });
  }


  protected async Task<IMessage?> HandleExceptions(Func<Task<IMessage?>> fn, IQuicRpcServiceContext ctx, long msgId,
    CancellationToken cancellationToken)
    => await HandleExceptions(fn(), ctx, msgId, cancellationToken);

  protected async Task<IMessage?> HandleExceptions(Task<IMessage?> task, IQuicRpcServiceContext ctx, long msgId,
    CancellationToken cancellationToken)
  {
    try
    {
      return await task;
    }
    catch (UnauthorizedAccessException ex)
    {
      return UnauthorizedReply(ctx, msgId, ex);
    }
    catch (OperationCanceledException ex) when (cancellationToken.IsCancellationRequested)
    {
      return TimedOutReply(ctx, msgId, ex);
    }
    catch (NotImplementedException ex)
    {
      return NotImplementedExceptionReply(ctx, msgId, ex);
    }
#if NET5_0_OR_GREATER
    catch (HttpRequestException ex)
    {
      return UnhandledHttpExceptionReply(ctx, msgId, ex);
    }
#endif
    catch (Exception ex)
    {
      return UnhandledExceptionReply(ctx, msgId, ex);
    }
  }
  protected IMessage FinalControlReply(IQuicRpcServiceContext ctx, long msgId, long errorCode, ReadOnlySpan<byte> message)
  {
    var response = new ReplyMessage(ctx, MessageType.FinalControl, new((nuint)(8 + message.Length + 1)));
    WriteErrorCode(response, errorCode);
    Append(response, message);
    WriteNullByte(response);
    return response;
  }
  private void Append(IMessage reply, ReadOnlySpan<byte> data)
  {
    if (data.IsEmpty) return;

    var prevSize = reply.Body.Length;
    var dataSize = (uint)data.Length;
    reply.Raw.Resize(prevSize + dataSize);
    data.CopyTo(reply.Raw.BigSpan.Slice(reply.HeaderSize));
  }
  private void Append(IMessage reply, string? data, bool nullTerminate = false)
  {
    if (data is null || data.Length == 0)
      return;

    var utf8 = Encoding.UTF8;

    var prevSize = reply.Body.Length;
    var dataSize = (uint)utf8.GetByteCount(data);
    reply.Raw.Resize(prevSize + dataSize + (nullTerminate ? 1u : 0u));
#if NETSTANDARD2_0
    var dataLen = data.Length;
    unsafe
    {
      fixed (byte* pRaw = reply.Body)
      {
        var p = pRaw + prevSize;
        fixed (char* pData = data)
          p += utf8.GetBytes(pData, dataLen, p, (int)dataSize);
        if (nullTerminate)
          *p = 0;
      }
    }
#else
    utf8.GetBytes(data, (Span<byte>)reply.Body.Slice(prevSize));
#endif
  }
  private void AddReplyExceptionMessage(IMessage reply, Exception? ex = null)
  {
    if (ex is null) return;
    var exTypeName = ex.GetType().Name;
    var exMessage = ex.Message;

    var utf8 = Encoding.UTF8;

    var prevSize = reply.Body.Length;
    var exTypeNameSize = (uint)utf8.GetByteCount(exTypeName);
    var exMessageSize = (uint)utf8.GetByteCount(exMessage);
    var exMsgSize = exTypeNameSize + exMessageSize + 3u;
    reply.Raw.Resize(prevSize + exMsgSize);
#if NETSTANDARD2_0
    var exTypeNameLen = exTypeName.Length;
    var exMessageLen = exMessage.Length;
    unsafe
    {
      fixed (byte* pRaw = reply.Body)
      {
        var p = pRaw + prevSize;
        fixed (char* pExTypeName = exTypeName)
          p += utf8.GetBytes(pExTypeName, exTypeNameLen, p, (int)exTypeNameSize);
        *p++ = (byte)':';
        *p++ = (byte)' ';
        fixed (char* pExMessage = exMessage)
          p += utf8.GetBytes(pExMessage, exMessageLen, p, (int)exMessageSize);
        *p = 0;
      }
    }
#else
    var slice = (Span<byte>)reply.Body.Slice(prevSize);
    slice = slice.Slice(utf8.GetBytes(exTypeName, slice));
    slice[0] = (byte)':';
    slice[1] = (byte)':';
    slice = slice.Slice(2);
    slice.Slice(utf8.GetBytes(exTypeName, slice));
    slice[0] = 0;
#endif
  }

  protected IMessage BadRequestReply(IQuicRpcServiceContext ctx, long msgId, Exception? ex = null)
  {
    var reply = FinalControlReply(ctx, msgId, 400, Utf8ErrorBadRequest);
#if DEBUG
    AddReplyExceptionMessage(reply, ex);
#endif
    return reply;
  }

  protected IMessage UnauthorizedReply(IQuicRpcServiceContext ctx, long msgId, Exception? ex = null)
  {
    var reply = FinalControlReply(ctx, msgId, 401, Utf8ErrorUnauthorized);
#if DEBUG
    AddReplyExceptionMessage(reply, ex);
#endif
    return reply;
  }

  protected IMessage NotFoundReply(IQuicRpcServiceContext ctx, long msgId, Exception? ex = null)
  {
    var reply = FinalControlReply(ctx, msgId, 404, Utf8ErrorNotFound);
#if DEBUG
    AddReplyExceptionMessage(reply, ex);
#endif
    return reply;
  }

  protected IMessage TimedOutReply(IQuicRpcServiceContext ctx, long msgId, Exception? ex = null)
  {
    var reply = FinalControlReply(ctx, msgId, 408, Utf8ErrorRequestTimeout);
#if DEBUG
    AddReplyExceptionMessage(reply, ex);
#endif
    return reply;
  }

  protected IMessage GoneReply(IQuicRpcServiceContext ctx, long msgId, Exception? ex = null)
  {
    var reply = FinalControlReply(ctx, msgId, 410, Utf8ErrorGone);
#if DEBUG
    AddReplyExceptionMessage(reply, ex);
#endif
    return reply;
  }

  protected IMessage NotImplementedExceptionReply(IQuicRpcServiceContext ctx, long msgId, Exception? ex = null)
  {
    var reply = FinalControlReply(ctx, msgId, 501, Utf8ErrorNotImplemented);
#if DEBUG
    AddReplyExceptionMessage(reply, ex);
#endif
    return reply;
  }

#if NET5_0_OR_GREATER
  protected IMessage UnhandledHttpExceptionReply(IQuicRpcServiceContext ctx, long msgId, HttpRequestException ex)
  {
    // ReSharper disable once ConstantNullCoalescingCondition
    ex ??= new("An exception was not provided.");
    var statusCode = ex.StatusCode ?? HttpStatusCode.InternalServerError;
    var message = RxSplitPascalCase.Replace(statusCode.ToString(), " $1");
    var reply = FinalControlReply(ctx, msgId, (long)statusCode, Encoding.UTF8.GetBytes(message));
#if DEBUG
    AddReplyExceptionMessage(reply, ex);
#endif
    return reply;
  }
#endif

  protected IMessage UnhandledExceptionReply(IQuicRpcServiceContext ctx, long msgId, Exception ex)
  {
    var reply = FinalControlReply(ctx, msgId, 500, Utf8ErrorInternalServerError);
#if DEBUG
    AddReplyExceptionMessage(reply, ex);
#endif
    return reply;
  }

  protected virtual Task<IMessage?> DispatchUnary<TMethodEnum>(
    TMethodEnum method,
    long sourceMsgId,
    ByteBuffer sourceByteBuffer,
    CancellationToken cancellationToken
  ) => Task.FromResult<IMessage?>(null);

  protected virtual Task<IMessage?> DispatchClientStreaming<TMethodEnum>(
    TMethodEnum method,
    long sourceMsgId,
    AsyncProducerConsumerCollection<IMessage> reader,
    CancellationToken cancellationToken
  ) => Task.FromResult<IMessage?>(null);

  protected virtual Task DispatchServerStreaming<TMethodEnum>(
    TMethodEnum method,
    long sourceMsgId,
    ByteBuffer sourceByteBuffer,
    CancellationToken cancellationToken
  ) => Task.CompletedTask;

  protected virtual Task DispatchStreaming<TMethodEnum>(
    TMethodEnum method,
    long sourceMsgId,
    AsyncProducerConsumerCollection<IMessage> reader,
    CancellationToken cancellationToken
  ) => Task.CompletedTask;

  protected abstract ReadOnlySpan<byte> ResolveMethodSignature<TMethodEnum>(TMethodEnum method) where TMethodEnum : Enum;

  protected abstract RpcMethodType ResolveMethodType<TMethodEnum>(TMethodEnum method) where TMethodEnum : Enum;

  /// <summary>
  /// Listens for and processes incoming procedure calls into method invocations. 
  /// </summary>
  /// <remarks>
  /// Should implement by invoking <see cref="RunAsync{TMethodEnum}"/>.
  /// </remarks>
  public abstract Task RunAsync(CancellationToken cancellationToken);

  protected virtual IMessage? OnUnhandledMessage(IMessage msg, CancellationToken cancellationToken)
    => OnUnhandledMessage(cancellationToken);

  protected virtual IMessage? OnUnhandledMessage(CancellationToken cancellationToken)
    => null;

  protected static T Track<T>(T disposable, ICollection<IAsyncDisposable> collection) where T : IAsyncDisposable
  {
    collection.Add(disposable);
    return disposable;
  }

  protected ChannelReader<T> WrapReader<T>(AsyncProducerConsumerCollection<IMessage> r) where T : struct, IBigBufferEntity
    => new EntityQuicMsgChannelReader<T>(r, Logger);
}
