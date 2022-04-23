#nullable enable
using System;
using System.Buffers.Text;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.NetworkInformation;
using System.Runtime;
using System.Security.Cryptography.X509Certificates;
using System.Text.Unicode;
using System.Threading;
using System.Threading.Channels;
using System.Threading.Tasks;
using BigBuffers.Xpc;
using BigBuffers.Xpc.Quic;
using FluentAssertions;
using Generated;
using NUnit.Framework;
using NUnit.Framework.Internal;
using StirlingLabs.MsQuic;
using StirlingLabs.Utilities;
using StirlingLabs.Utilities.Collections;

namespace BigBuffers.Tests;

[SingleThreaded]
public class QuicRpcServiceTests {

  static QuicRpcServiceTests() {
    ProfileOptimization.SetProfileRoot(Environment.CurrentDirectory);
    ProfileOptimization.StartProfile(nameof(QuicRpcServiceTests));
  }

  private static double TimeStamp => SharedCounters.GetTimeSinceStarted().TotalSeconds;

  private static int _lastIssuedFreeEphemeralTcpPort = -1;

  private static readonly Type? QuicConnectionAbortedExceptionType =
    Type.GetType(
      "System.Net.Quic.QuicConnectionAbortedException, System.Net.Quic, Version=6.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a");

  private static readonly Type? QuicOperationAbortedExceptionType =
    Type.GetType(
      "System.Net.Quic.QuicOperationAbortedException, System.Net.Quic, Version=6.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a");

  private static int GetFreeEphemeralTcpPort() {
    bool IsFree(int realPort) {
      var properties = IPGlobalProperties.GetIPGlobalProperties();
      var listeners = properties.GetActiveTcpListeners();
      var openPorts = listeners.Select(item => item.Port).ToArray();
      return openPorts.All(openPort => openPort != realPort);
    }

    const int ephemeralRangeSize = 16384;
    const int ephemeralRangeStart = 49152;

    var port = (_lastIssuedFreeEphemeralTcpPort + 1) % ephemeralRangeSize;

    while (!IsFree(ephemeralRangeStart + port))
      port = (port + 1) % ephemeralRangeSize;

    _lastIssuedFreeEphemeralTcpPort = port;

    return ephemeralRangeStart + port;
  }

  // TODO: confirm client and server can identify when a stream or connection is unexpectedly interrupted or terminated

  [Test]
  //[Timeout(15000)]
  [Theory]
  [SuppressMessage("ReSharper", "CompareOfFloatsByEqualityOperator")]
  [NonParallelizable]
  public async Task UnaryRoundTrip([Range(0, 10)] int run) {
    var isDebug = Debugger.IsAttached;
    var logger = run <= 1 || isDebug
      ? new DebugTestContextWriter(TestContext.Out)
      : null; //TestContext.Out;

    TaskScheduler.UnobservedTaskException += (_, args) => {
      var aex = args.Exception;
      if (aex.InnerExceptions.Count == 1) {
        var ex = aex.InnerException;
        var exType = ex.GetType();
        if (exType.IsAssignableTo(QuicConnectionAbortedExceptionType)
            || exType.IsAssignableTo(QuicOperationAbortedExceptionType))
          return;
      }

      Debugger.Break();
    };

    //var bigDelays = run <= 5;

    var testTimeoutMs = 5000;
    var cts =
      Debugger.IsAttached ? new() : new CancellationTokenSource(testTimeoutMs);

    var testName = TestContext.CurrentContext.Test.FullName;
    using var reg = new QuicRegistration(testName);
    using var listenerCfg = new QuicServerConfiguration(reg, "Test");
    using var listener = new QuicListener(listenerCfg);
    var asmDir = Path.GetDirectoryName(new Uri(typeof(QuicRpcServiceTests).Assembly.Location).LocalPath)!;
    var p12Path = Path.Combine(asmDir, "localhost.p12");
    var cert = new QuicCertificate(policy => {
      policy.RevocationMode = X509RevocationMode.NoCheck;
      policy.DisableCertificateDownloads = false;
      policy.VerificationFlags |= X509VerificationFlags.AllowUnknownCertificateAuthority
        | X509VerificationFlags.IgnoreCertificateAuthorityRevocationUnknown
        | X509VerificationFlags.IgnoreCtlSignerRevocationUnknown
        | X509VerificationFlags.IgnoreRootRevocationUnknown
        | X509VerificationFlags.IgnoreEndRevocationUnknown;
    }, File.OpenRead(p12Path));
    listenerCfg.ConfigureCredentials(cert);

    using var server = new QuicRpcServiceTestImpl.QuicRpcServiceTestServerImpl("Test", listener, logger);

    var port = GetFreeEphemeralTcpPort();

    var ep = new IPEndPoint(IPAddress.IPv6Any, port);

    using var clientCfg = new QuicClientConfiguration(reg, "Test");
    clientCfg.ConfigureCredentials();

    using var clientConnection = new QuicClientConnection(clientCfg);

    listener.Start(ep);

    try {
      var client = new QuicRpcServiceTestImpl.QuicRpcServiceTestClientImpl("Test", clientConnection, logger);

      await clientConnection.ConnectAsync("::1", (ushort)ep.Port);

      TestContext.WriteLine("Building message message to send");
      var bb = new BigBufferBuilder();
      Message.StartMessage(bb);
      Message.AddSubject(bb, bb.MarkStringPlaceholder(out var subj));
      Message.AddBody(bb, bb.MarkOffsetPlaceholder(out Placeholder<Empty> e).Value);
      Message.AddBodyType(bb, MessageBody.Empty);
      Message.EndMessage(bb);
      Empty.StartEmpty(bb);
      e.Fill(Empty.EndEmpty(bb));
      subj.Fill("Hello");

      TestContext.WriteLine("Sending hello message");

      var ct = cts.Token;

      var testExecCtx = TestExecutionContext.CurrentContext;

      var result = await client.Unary(new(0, bb.ByteBuffer), ct);

      TestContext.WriteLine("Received a message");

      result.Subject.Should().Be("RE: Hello");

      TestContext.WriteLine("Received \"Hello\" successfully");
    }
    finally {
      TestContext.WriteLine("Shutting down...");
      clientConnection.Close();
      listener.Stop();
      reg.Shutdown(0);
    }

    TestContext.WriteLine("Disposing...");
  }

  private static readonly byte[] _clientStreamingReplyExpected =
    Convert.FromBase64String(
      "4P////////84AAAAAAAAACAAAAAAAAAAAwAAAAAAAA"
      + "AKACAACAAYABAAAAAAAAAA+P////////8EAAgAAAAA"
      + "ABAAAAAAAAAAUkU6IEhlbGxvLCBIZWxsbwAAAAAAAA"
      + "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
      + "AAA=");

  [Test]
  //[Timeout(15000)]
  [Theory]
  [SuppressMessage("ReSharper", "CompareOfFloatsByEqualityOperator")]
  [NonParallelizable]
  public async Task ClientStreamingRoundTrip([Range(0, 10)] int run) {
    var isDebug = Debugger.IsAttached;
    var logger = run <= 1 || isDebug
      ? new DebugTestContextWriter(TestContext.Out)
      : null; //TestContext.Out;

    TaskScheduler.UnobservedTaskException += (_, args) => {
      var aex = args.Exception;
      if (aex.InnerExceptions.Count == 1) {
        var ex = aex.InnerException;
        var exType = ex.GetType();
        if (exType.IsAssignableTo(QuicConnectionAbortedExceptionType)
            || exType.IsAssignableTo(QuicOperationAbortedExceptionType))
          return;
      }

      Debugger.Break();
    };

    //var bigDelays = run <= 5;

    var testTimeoutMs = 5000;
    var cts =
      Debugger.IsAttached ? new() : new CancellationTokenSource(testTimeoutMs);

    var testName = TestContext.CurrentContext.Test.FullName;
    using var reg = new QuicRegistration(testName);
    using var listenerCfg = new QuicServerConfiguration(reg, "Test");
    using var listener = new QuicListener(listenerCfg);
    var asmDir = Path.GetDirectoryName(new Uri(typeof(QuicRpcServiceTests).Assembly.Location).LocalPath)!;
    var p12Path = Path.Combine(asmDir, "localhost.p12");
    var cert = new QuicCertificate(policy => {
      policy.RevocationMode = X509RevocationMode.NoCheck;
      policy.DisableCertificateDownloads = false;
      policy.VerificationFlags |= X509VerificationFlags.AllowUnknownCertificateAuthority
        | X509VerificationFlags.IgnoreCertificateAuthorityRevocationUnknown
        | X509VerificationFlags.IgnoreCtlSignerRevocationUnknown
        | X509VerificationFlags.IgnoreRootRevocationUnknown
        | X509VerificationFlags.IgnoreEndRevocationUnknown;
    }, File.OpenRead(p12Path));
    listenerCfg.ConfigureCredentials(cert);

    using var server = new QuicRpcServiceTestImpl.QuicRpcServiceTestServerImpl("Test", listener, logger);

    var port = GetFreeEphemeralTcpPort();

    var ep = new IPEndPoint(IPAddress.IPv6Any, port);

    using var clientCfg = new QuicClientConfiguration(reg, "Test");
    clientCfg.ConfigureCredentials();

    using var clientConnection = new QuicClientConnection(clientCfg);

    listener.Start(ep);

    try {
      var client = new QuicRpcServiceTestImpl.QuicRpcServiceTestClientImpl("Test", clientConnection, logger);

      await clientConnection.ConnectAsync("::1", (ushort)ep.Port);

      TestContext.WriteLine("Building message message to send");
      var bb = new BigBufferBuilder();
      Message.StartMessage(bb);
      Message.AddSubject(bb, bb.MarkStringPlaceholder(out var subj));
      Message.AddBody(bb, bb.MarkOffsetPlaceholder(out Placeholder<Empty> e).Value);
      Message.AddBodyType(bb, MessageBody.Empty);
      Message.EndMessage(bb);
      Empty.StartEmpty(bb);
      e.Fill(Empty.EndEmpty(bb));
      subj.Fill("Hello");

      var ct = cts.Token;

      var testExecCtx = TestExecutionContext.CurrentContext;

      var messageReader = new EnumerableChannelReader<Message>(new Message[] { new(0, bb.ByteBuffer), new(0, bb.ByteBuffer) });

      TestContext.WriteLine("Sending 2 Hello messages");

      var result = await client.ClientStreaming(messageReader, ct);

      TestContext.WriteLine("Received a message");
      
      void X() {
        var resultBytes = ((IBigBufferTable)result).Model.ByteBuffer.ToSizedReadOnlySpan();
        #if NET5_0_OR_GREATER
        TestContext.WriteLine("E0FFFFFFFFFFFFFF3800000000000000200000000000000003000000000000000A002000080018001000000000000000F8FFFFFFFFFFFFFF0400080000000000100000000000000052453A2048656C6C6F2C2048656C6C6F00000000000000000000000000000000000000000000000000000000000000000000000000000000");
        TestContext.WriteLine(Convert.ToHexString((ReadOnlySpan<byte>) resultBytes));
        #endif
        Assert.IsTrue(resultBytes.CompareMemory(_clientStreamingReplyExpected) == 0, "Reply should be uniform.");
      }

      X();

      result.Subject.Should().Be("RE: Hello, Hello");

      TestContext.WriteLine("Received \"Hello\" successfully");
    }
    finally {
      TestContext.WriteLine("Shutting down...");
      clientConnection.Close();
      listener.Stop();
      reg.Shutdown(0);
    }

    TestContext.WriteLine("Disposing...");
  }

  public static class QuicRpcServiceTestImpl {

    private enum Method : long {

      Unary = 1,

      ClientStreaming = 2,

      ServerStreaming = 3,

      BidirectionalStreaming = 4

    }

    private static readonly SizedUtf8String SignatureUnary = "Unary";

    private static readonly SizedUtf8String SignatureClientStreaming = "ClientStreaming";

    private static readonly SizedUtf8String SignatureServerStreaming = "ServerStreaming";

    private static readonly SizedUtf8String SignatureBidirectionalStreaming = "BidirectionalStreaming";

    private static SizedUtf8String StaticResolveMethodSignature(Method method)
      => method switch {
        Method.Unary => SignatureUnary,
        Method.ClientStreaming => SignatureClientStreaming,
        Method.ServerStreaming => SignatureServerStreaming,
        Method.BidirectionalStreaming => SignatureBidirectionalStreaming,
        _ => throw new ArgumentOutOfRangeException(nameof(method))
      };

    public sealed class QuicRpcServiceTestClientImpl : QuicRpcServiceClientBase {

      public QuicRpcServiceTestClientImpl(SizedUtf8String name, QuicPeerConnection connection, TextWriter? logger = null)
        : base(name, connection, logger) {
      }

      protected override SizedUtf8String ResolveMethodSignature<TMethodEnum>(TMethodEnum method) {
        if (method is not Method m)
          throw new InvalidOperationException();

        return StaticResolveMethodSignature(m);
      }

      public async Task<Message> Unary(Message message, CancellationToken ct = default)
        => await UnaryRequest<Method, Message, Message>(Method.Unary, message, ct);

      public async Task<Message> ClientStreaming(ChannelReader<Message> messages, CancellationToken ct = default)
        => await ClientStreamingRequest<Method, Message, Message>(Method.ClientStreaming, messages, ct);

      public async Task ServerStreaming(Message m, ChannelWriter<Message> writer, CancellationToken ct = default)
        => await ServerStreamingRequest<Method, Message, Message>(Method.ServerStreaming, m, ct).WriteTo(writer, ct);

      public async Task BidirectionalStreaming(ChannelReader<Message> messages, ChannelWriter<Message> writer, CancellationToken ct = default)
        => await StreamingRequest<Method, Message, Message>(Method.BidirectionalStreaming, messages, ct).WriteTo(writer, ct);

    }

    public sealed class QuicRpcServiceTestServerImpl : QuicRpcServiceServerBase {

      public QuicRpcServiceTestServerImpl(string name, QuicListener listener, TextWriter? logger = null)
        : base(name, listener, logger) {
      }

      protected override SizedUtf8String ResolveMethodSignature<TMethodEnum>(TMethodEnum method) {
        if (method is not Method m)
          throw new InvalidOperationException();

        return StaticResolveMethodSignature(m);
      }

      protected override RpcMethodType ResolveMethodType<TMethodEnum>(TMethodEnum method) {
        if (method is not Method m)
          throw new InvalidOperationException();

        return m switch {
          Method.Unary => RpcMethodType.Unary,
          Method.ClientStreaming => RpcMethodType.ClientStreaming,
          Method.ServerStreaming => RpcMethodType.ServerStreaming,
          Method.BidirectionalStreaming => RpcMethodType.BidirectionalStreaming,
          _ => throw new ArgumentOutOfRangeException(nameof(method))
        };
      }

      public override Task RunAsync(CancellationToken cancellationToken)
        => RunAsync<Method>(cancellationToken);

      public async Task RunAsync<TMethodEnum>(CancellationToken cancellationToken) {
      }

      public override async Task Dispatch(IMessage sourceMsg, CancellationToken ct = default)
        => await Dispatch<Method>(sourceMsg, ct);

      protected override async Task<IMessage?> DispatchUnary<TMethodEnum>(
        TMethodEnum method,
        long sourceMsgId,
        ByteBuffer sourceByteBuffer,
        CancellationToken cancellationToken
      ) {
        if (method is not Method m) throw new InvalidOperationException();

        return m switch {
          Method.Unary => await Reply(Unary(new(0, sourceByteBuffer), cancellationToken)),
          _ => throw new ArgumentOutOfRangeException(nameof(method))
        };
      }

      protected override async Task<IMessage?> DispatchClientStreaming<TMethodEnum>(
        TMethodEnum method,
        long sourceMsgId,
        AsyncProducerConsumerCollection<IMessage> reader,
        CancellationToken cancellationToken
      ) {
        if (method is not Method m) throw new InvalidOperationException();

        return m switch {
          Method.ClientStreaming => await Reply(ClientStreaming(WrapReader<@Message>(reader), cancellationToken)),
          _ => throw new ArgumentOutOfRangeException(nameof(method))
        };
      }

      protected override Task DispatchServerStreaming<TMethodEnum>(TMethodEnum method,
        long sourceMsgId,
        QuicRpcServiceServerContext ctx,
        ByteBuffer sourceByteBuffer,
        CancellationToken cancellationToken) {
        if (method is not Method m) throw new InvalidOperationException();

        var ads = new List<IAsyncDisposable>(1);

        async Task CleanUpContinuation(Task _) {
          foreach (var ad in ads) await ad.DisposeAsync();
        }

        Task CleanUpAfter(Task t) => t.ContinueWith(CleanUpContinuation);

        ChannelWriter<T> Writer<T>() where T : struct, IBigBufferEntity
          => Track(new QuicMsgStreamWriter<T>(ctx, sourceMsgId, Logger), ads);

        return m switch {
          Method.ServerStreaming => CleanUpAfter(ServerStreaming(new(0, sourceByteBuffer), Writer<@Message>(), cancellationToken)),
          _ => throw new ArgumentOutOfRangeException(nameof(method))
        };
      }

      protected override Task DispatchStreaming<TMethodEnum>(TMethodEnum method,
        long sourceMsgId,
        QuicRpcServiceServerContext ctx,
        AsyncProducerConsumerCollection<IMessage> reader,
        CancellationToken cancellationToken) {
        if (method is not Method m) throw new InvalidOperationException();

        var ads = new List<IAsyncDisposable>(1);

        async Task CleanUpContinuation(Task _) {
          foreach (var ad in ads) await ad.DisposeAsync();
        }

        Task CleanUpAfter(Task t) => t.ContinueWith(CleanUpContinuation, default(CancellationToken));

        ChannelWriter<T> Writer<T>() where T : struct, IBigBufferEntity
          => Track(new QuicMsgStreamWriter<T>(ctx, sourceMsgId, Logger), ads);

        return m switch {
          Method.BidirectionalStreaming => CleanUpAfter(BidirectionalStreaming(WrapReader<@Message>(reader),
            Writer<@Message>(), cancellationToken)),
          _ => throw new ArgumentOutOfRangeException(nameof(method))
        };
      }

      // below is non-generated user impl

      public async Task<Message> Unary(Message m, CancellationToken ct = default) {
        var bb = new BigBufferBuilder();
        Message.StartMessage(bb);
        Message.AddSubject(bb, bb.MarkStringPlaceholder(out var subj));
        Message.AddBody(bb, bb.MarkOffsetPlaceholder(out Placeholder<Empty> e).Value);
        Message.AddBodyType(bb, MessageBody.Empty);
        var reply = Message.EndMessage(bb).Resolve(bb);
        Empty.StartEmpty(bb);
        e.Fill(Empty.EndEmpty(bb));
        subj.Fill("RE: " + m.Subject);
        return reply;
      }

      public async Task<Message> ClientStreaming(ChannelReader<Message> msgs, CancellationToken ct = default) {
        var subjects = new List<string>();
        await foreach (var msg in msgs.AsConsumingAsyncEnumerable(ct))
          subjects.Add(msg.Subject);

        var bb = new BigBufferBuilder();
        Message.StartMessage(bb);
        Message.AddSubject(bb, bb.MarkStringPlaceholder(out var subj));
        Message.AddBody(bb, bb.MarkOffsetPlaceholder(out Placeholder<Empty> e).Value);
        Message.AddBodyType(bb, MessageBody.Empty);
        var reply = Message.EndMessage(bb).Resolve(bb);
        Empty.StartEmpty(bb);
        e.Fill(Empty.EndEmpty(bb));
        subj.Fill($"RE: {string.Join(", ", subjects)}");
        return reply;
      }

      public async Task ServerStreaming(Message m, ChannelWriter<Message> writer, CancellationToken ct = default) {
        try {
          Logger?.WriteLine(
            $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: ServerStreaming started");

          var subject = m.Subject;

          {
            var bb = new BigBufferBuilder();
            Message.StartMessage(bb);
            Message.AddSubject(bb, bb.MarkStringPlaceholder(out var subj));
            Message.AddBody(bb, bb.MarkOffsetPlaceholder(out Placeholder<Empty> e).Value);
            Message.AddBodyType(bb, MessageBody.Empty);
            var reply = Message.EndMessage(bb).Resolve(bb);
            Empty.StartEmpty(bb);
            e.Fill(Empty.EndEmpty(bb));
            subj.Fill($"RE: {subject}");

            Logger?.WriteLine(
              $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: ServerStreaming delaying a bit");
            await Task.Delay(1, ct);

            Logger?.WriteLine(
              $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: ServerStreaming writing reply 1");
            await writer.WriteAsync(reply, ct);
          }

          {
            var bb = new BigBufferBuilder();
            Message.StartMessage(bb);
            Message.AddSubject(bb, bb.MarkStringPlaceholder(out var subj));
            Message.AddBody(bb, bb.MarkOffsetPlaceholder(out Placeholder<Empty> e).Value);
            Message.AddBodyType(bb, MessageBody.Empty);
            var reply = Message.EndMessage(bb).Resolve(bb);
            Empty.StartEmpty(bb);
            e.Fill(Empty.EndEmpty(bb));
            subj.Fill($"RE Ctd.: {subject}");

            Logger?.WriteLine(
              $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: ServerStreaming delaying a bit");
            await Task.Delay(1, ct);

            Logger?.WriteLine(
              $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: ServerStreaming writing reply 2");
            await writer.WriteAsync(reply, ct);
          }

          Logger?.WriteLine(
            $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: ServerStreaming finished");
        }
        finally {
          Logger?.WriteLine(
            $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: ServerStreaming completing");
          writer.Complete();
          Logger?.WriteLine(
            $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: ServerStreaming completed");
        }
      }

      public async Task BidirectionalStreaming(ChannelReader<Message> msgs, ChannelWriter<Message> writer, CancellationToken ct = default) {
        try {
          Logger?.WriteLine(
            $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: BidirectionalStreaming started");

          await foreach (var msg in msgs.AsConsumingAsyncEnumerable(ct)) {
            var subject = msg.Subject;
            Logger?.WriteLine(
              $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: BidirectionalStreaming got {subject}");

            var bb = new BigBufferBuilder();
            Message.StartMessage(bb);
            Message.AddSubject(bb, bb.MarkStringPlaceholder(out var subj));
            Message.AddBody(bb, bb.MarkOffsetPlaceholder(out Placeholder<Empty> e).Value);
            Message.AddBodyType(bb, MessageBody.Empty);
            var reply = Message.EndMessage(bb).Resolve(bb);
            Empty.StartEmpty(bb);
            e.Fill(Empty.EndEmpty(bb));
            subj.Fill($"RE: {subject}");

            Logger?.WriteLine(
              $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: BidirectionalStreaming delaying a bit");
            await Task.Delay(1, ct);

            await writer.WriteAsync(reply, ct);
            Logger?.WriteLine(
              $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: BidirectionalStreaming wrote RE: {subject}");
          }

          Logger?.WriteLine(
            $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: BidirectionalStreaming finished");
        }
        finally {
          Logger?.WriteLine(
            $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: BidirectionalStreaming completing");
          writer.Complete();
          Logger?.WriteLine(
            $"[{TimeStamp:F3}] {GetType().Name} T{Task.CurrentId}: BidirectionalStreaming completed");
        }
      }

    }

  }

}
