using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;

namespace CppRest
{
    public class ProxyHelper
    {
        public static IWebProxy GetProxyNow(IExtProxyInfo proxySettings)
        {
            IWebProxy proxy = null;

            if (!proxySettings.ProxyEnabled)
            {
                proxy = WebRequest.GetSystemWebProxy();
                proxy.Credentials = CredentialCache.DefaultNetworkCredentials;
                return proxy;
            }

            if (proxySettings.Using == ExtProxyUsing.DontUseProxy)
                return null;

            if (proxySettings.Using == ExtProxyUsing.UseSystemProxyAddress)
            {
                proxy = WebRequest.GetSystemWebProxy();
                proxy.Credentials = null;
            }
            else
                proxy = proxySettings.ProxyPort.HasValue 
                    ? new WebProxy(proxySettings.ProxyServer, proxySettings.ProxyPort.Value) 
                    : new WebProxy(proxySettings.ProxyServer);

            if (!proxySettings.ProxyAuthorizationEnabled)
                return proxy;

            var domain = proxySettings.ProxyDomain;
            proxy.Credentials = string.IsNullOrEmpty(domain) 
                ? new NetworkCredential(proxySettings.ProxyLogin, proxySettings.ProxyPassword)
                : new NetworkCredential(proxySettings.ProxyLogin, proxySettings.ProxyPassword, domain);

            return proxy;
        }

        public static IWebProxy GetProxyCached(IExtProxyInfo proxySettings)
        {
            return ExtProxies.GetOrAdd(proxySettings, GetProxyNow);
        }

        private static readonly ConcurrentDictionary<IExtProxyInfo, IWebProxy> ExtProxies = new ConcurrentDictionary<IExtProxyInfo, IWebProxy>();
    }
}
