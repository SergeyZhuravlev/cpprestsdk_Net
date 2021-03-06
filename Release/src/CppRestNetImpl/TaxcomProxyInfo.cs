﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;

namespace CppRest
{
    public enum TaxcomProxyUsing
    {
        DefaultBehavior = 0,
        DontUseProxy = 1,
        UseSystemProxyAddress = 2
    }

    public interface ITaxcomProxyInfo: IEquatable<ITaxcomProxyInfo>
    {
        TaxcomProxyUsing Using { get; }
        bool ProxyEnabled { get; }
        string ProxyServer { get; }
        int? ProxyPort { get; }
        bool ProxyAuthorizationEnabled { get; }
        string ProxyDomain { get; }
        string ProxyLogin { get; }
        string ProxyPassword { get; }
    }

    public abstract class TaxcomProxyInfoBase : ITaxcomProxyInfo
    {
        public bool Equals(ITaxcomProxyInfo other)
        {
            return (!(other is null) 
                    && Tuple.Create(Using, ProxyEnabled, ProxyServer, ProxyPort,
                        ProxyAuthorizationEnabled, ProxyDomain, ProxyLogin, ProxyPassword)
                    .Equals(Tuple.Create(other.Using, other.ProxyEnabled, other.ProxyServer, other.ProxyPort,
                        other.ProxyAuthorizationEnabled, other.ProxyDomain, other.ProxyLogin, other.ProxyPassword)));
        }

        public override int GetHashCode()
        {
            return Tuple.Create(Using, ProxyEnabled, ProxyServer, ProxyPort, ProxyAuthorizationEnabled, ProxyDomain, ProxyLogin, ProxyPassword).GetHashCode();
        }

        public abstract TaxcomProxyUsing Using { get; }
        public abstract bool ProxyEnabled { get; }
        public abstract string ProxyServer { get; }
        public abstract int? ProxyPort { get; }
        public abstract bool ProxyAuthorizationEnabled { get; }
        public abstract string ProxyDomain { get; }
        public abstract string ProxyLogin { get; }
        public abstract string ProxyPassword { get; }
    }
}
