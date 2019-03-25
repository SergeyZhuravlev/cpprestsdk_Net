using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading;

namespace CppRestNetImpl
{
    public static class InvokeAborter
    {
        public static R InvokeWithCancellation<R>(Func<R> invokable, CancellationToken token, Action aborter)
        {
            token.ThrowIfCancellationRequested();
            R result;
            using (token.Register(aborter))
                result = invokable();
            token.ThrowIfCancellationRequested();
            return result;
        }

        public static void InvokeWithCancellation(Action invokable, CancellationToken token, Action aborter)
        {
            token.ThrowIfCancellationRequested();
            using (token.Register(aborter))
                invokable();
            token.ThrowIfCancellationRequested();
        }

        public static R WebRequestInvokeWithCancellation<R>(Func<R> invokable, CancellationToken token, WebRequest aborting)
        {
            token.ThrowIfCancellationRequested();
            R result;
            try
            {
                result = InvokeWithCancellation(invokable, token, aborting.Abort);
            }
            catch (WebException)
            {
                token.ThrowIfCancellationRequested();
                throw;
            }
            token.ThrowIfCancellationRequested();
            return result;
        }

        public static void WebRequestInvokeWithCancellation(Action invokable, CancellationToken token, WebRequest aborting)
        {
            token.ThrowIfCancellationRequested();
            try
            {
                InvokeWithCancellation(invokable, token, aborting.Abort);
            }
            catch (WebException)
            {
                token.ThrowIfCancellationRequested();
                throw;
            }
            token.ThrowIfCancellationRequested();
        }
    }
}
