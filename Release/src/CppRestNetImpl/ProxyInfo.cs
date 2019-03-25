using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;

namespace CppRest
{
    public enum ProxyMode
    {
        IE = 0,
        Manual = 1,
        Disabled = 2
    }

    public interface ICredentials
    {
        string ProxyDomainWithLogin { get; }
        string ProxyPassword { get; }
    }

    public class Credentials: ICredentials
    {
        public string ProxyDomainWithLogin { get; set; }
        public string ProxyPassword { get; set; }
    }

    public interface IProxyInfo
    {
        ProxyMode Mode { get; }
        ICredentials Credential { get; }
        string ProxyUriHostWithPort { get; }
    }

    public class ProxyInfo: TaxcomProxyInfoBase, IProxyInfo
    {
        public ProxyInfo(ProxyMode mode, ICredentials credential, string proxyUriHostWithPort)
        {
            this.Mode = mode;
            this.Credential = credential;
            this.ProxyUriHostWithPort = proxyUriHostWithPort;
            proxyUriHostWithPort = FixProxyUriHostWithPort(proxyUriHostWithPort);
            switch (mode)
            {
                case ProxyMode.IE:
                {
                    if (credential is null)
                    {
                        Using = TaxcomProxyUsing.DefaultBehavior;
                        ProxyEnabled = false;
                    }
                    else
                    {
                        Using = TaxcomProxyUsing.UseSystemProxyAddress;
                        ProxyEnabled = true;
                        ProxyAuthorizationEnabled = true;
                        ProxyLogin = GetLogin(credential.ProxyDomainWithLogin);
                        ProxyDomain = GetDomain(credential.ProxyDomainWithLogin);
                        ProxyPassword = credential.ProxyPassword;
                    }

                }
                    break;
                case ProxyMode.Disabled:
                {
                    Using = TaxcomProxyUsing.DontUseProxy;
                    ProxyEnabled = true;
                }
                    break;
                case ProxyMode.Manual:
                {
                    Using = TaxcomProxyUsing.DefaultBehavior;
                    var proxyEnabled = !string.IsNullOrEmpty(proxyUriHostWithPort);
                    if (proxyEnabled)
                    {
                        ProxyPort = GetPort(proxyUriHostWithPort);
                        ProxyServer = GetHost(proxyUriHostWithPort);
                    }
                    ProxyEnabled = proxyEnabled;
                    if (credential is null)
                        ProxyAuthorizationEnabled = false;
                    else
                    {
                        ProxyAuthorizationEnabled = true;
                        ProxyLogin = GetLogin(credential.ProxyDomainWithLogin);
                        ProxyDomain = GetDomain(credential.ProxyDomainWithLogin);
                        ProxyPassword = credential.ProxyPassword;
                    }
                }
                    break;
                default:
                    throw new NotImplementedException($"{nameof(mode)}:{mode} ({(int)mode})");
            }
        }

        private string FixProxyUriHostWithPort(string proxyUriHostWithPort)
        {
            if (string.IsNullOrEmpty(proxyUriHostWithPort))
                return null;
            if (proxyUriHostWithPort.StartsWith("/"))
                return "http:" + proxyUriHostWithPort;
            return proxyUriHostWithPort;
        }

        private int? GetPort(string proxyUriHostWithPort)
        {
            try
            {
                if (string.IsNullOrEmpty(proxyUriHostWithPort))
                    return null;
                var uri = new Uri(proxyUriHostWithPort);
                return uri.Port;
            }
            catch (Exception ex)
            {
                throw new ArgumentException($"{MethodBase.GetCurrentMethod()}: for value '{proxyUriHostWithPort}'", ex);
            }
        }

        private string GetHost(string proxyUriHostWithPort)
        {
            try
            {
                if (string.IsNullOrEmpty(proxyUriHostWithPort))
                    return null;
                var uri = new Uri(proxyUriHostWithPort);
                return uri.Host;
            }
            catch (Exception ex)
            {
                throw new ArgumentException($"{MethodBase.GetCurrentMethod()}: for value '{proxyUriHostWithPort}'", ex);
            }
}

        private string GetLogin(string credentialProxyDomainWithLogin)
        {
            if (string.IsNullOrEmpty(credentialProxyDomainWithLogin))
                return null;
            var splited = credentialProxyDomainWithLogin.Split('\\');
            return splited.Last();
        }

        private string GetDomain(string credentialProxyDomainWithLogin)
        {
            if (string.IsNullOrEmpty(credentialProxyDomainWithLogin))
                return null;
            var splited = credentialProxyDomainWithLogin.Split('\\');
            if (splited.Length > 1)
                return splited.First();
            else
                return null;
        }

        #region TaxcomProxyInfo
        public override TaxcomProxyUsing Using { get; }
        public override bool ProxyEnabled { get; }
        public override string ProxyServer { get; }
        public override int? ProxyPort { get; }
        public override bool ProxyAuthorizationEnabled { get; }
        public override string ProxyDomain { get; }
        public override string ProxyLogin { get; }
        public override string ProxyPassword { get; }
        #endregion

        #region IProxyInfo
        public ProxyMode Mode { get; }
        public ICredentials Credential { get; }
        public string ProxyUriHostWithPort { get; }
        #endregion
    }
}
