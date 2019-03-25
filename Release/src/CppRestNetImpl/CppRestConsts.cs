using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CppRestNetImpl
{
    public class CppRestConsts
    {
        public const int BufferSize = 4096;
        //public static readonly int DefaultTimeout = (int)(TimeSpan.FromMinutes(5).TotalMilliseconds);
        public static readonly string UserAgentHeader = "User-Agent";
        public static readonly string ContentLengthHeader = "Content-Length";
        public static readonly string ContentTypeHeader = "Content-Type";
        public static readonly string RangeHeader = "Range";

        public static readonly string AcceptHeader = "Accept";
        public static readonly string ConnectionHeader = "Connection";
        public static readonly string DateHeader = "Date";
        public static readonly string ExpectHeader = "Expect";
        public static readonly string HostHeader = "Host";
        public static readonly string IfModifiedSinceHeader = "If-Modified-Since";
        public static readonly string ProxyConnectionHeader = "Proxy-Connection";
        public static readonly string RefererHeader = "Referer";
        public static readonly string TransferEncodingHeader = "Transfer-Encoding";
    }
}
