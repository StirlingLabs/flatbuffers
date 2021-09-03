using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Channels;
using System.Threading.Tasks;
using JetBrains.Annotations;

namespace BigBuffers
{
  [PublicAPI]
  public static class ChannelExtensions
  {
    public static async IAsyncEnumerable<T> AsConsumingAsyncEnumerable<T>(this ChannelReader<T> reader,
      [EnumeratorCancellation] CancellationToken cancellationToken = default)
    {
      while (!cancellationToken.IsCancellationRequested && !reader.Completion.IsCompleted)
      {
        while (reader.TryRead(out var item))
          yield return item;

        if (cancellationToken.IsCancellationRequested || reader.Completion.IsCompleted)
          break;

        if (!await reader.WaitToReadAsync(cancellationToken))
          break;
      }

      Debug.Assert(reader.Completion.IsCompleted);
    }

    public static async Task WriteTo<T>(this IAsyncEnumerable<T> enumerable, ChannelWriter<T> writer,
      CancellationToken cancellationToken = default)
    {
      try
      {
        await using var enumerator = enumerable.GetAsyncEnumerator(cancellationToken);

        while (!cancellationToken.IsCancellationRequested)
        {
          if (!await enumerator.MoveNextAsync())
            break;

          if (cancellationToken.IsCancellationRequested)
            break;

          if (writer.TryWrite(enumerator.Current))
            continue;

          if (cancellationToken.IsCancellationRequested)
            break;

          if (!await writer.WaitToWriteAsync(cancellationToken))
            break;
        }
        writer.TryComplete();
      }
      catch (Exception ex)
      {
        try
        {
          writer.TryComplete(ex);
        }
        catch (ObjectDisposedException)
        {
          return;
        }
      }
    }
  }
}
