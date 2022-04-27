using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading;
using CppRest;

namespace CppRestNetImpl
{
    public interface IRequestSettings
    {
        IProxyInfo ProxyInfo { get; }
        IExtProxyInfo ExtProxyInfo { get; }
        TimeSpan? ReadTimeout { get; }
        TimeSpan? WriteTimeout { get; }
        TimeSpan? ConnectTimeout { get; }
        ICollection<KeyValuePair<string, StoreLocation>> ClientCertificates { get; }
        System.Net.ICredentials Credentials { get; }
    }

    public class RequestSettings: IRequestSettings
    {
        public IProxyInfo ProxyInfo { get; set; }
        public IExtProxyInfo ExtProxyInfo => ProxyInfo as IExtProxyInfo;

        public TimeSpan? ReadTimeout { get; set; }
        public TimeSpan? WriteTimeout { get; set; }
        public TimeSpan? ConnectTimeout { get; set; }
        public ICollection<KeyValuePair<string, StoreLocation>> ClientCertificates { get; } = new List<KeyValuePair<string, StoreLocation>>();
        public System.Net.ICredentials Credentials { get; set; }

        public TimeSpan? AllTimeouts
        {
            set
            {
                ReadTimeout = value;
                WriteTimeout = value;
                ConnectTimeout = value;
            }
        }
    }
}
