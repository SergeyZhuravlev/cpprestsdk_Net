using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.ExceptionServices;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Text.RegularExpressions;

namespace CppRestNetImpl
{
    public class CertificateHelper
    {
        public static X509Certificate2Collection GetCachedCertificateCollection(ICollection<KeyValuePair<string, StoreLocation>> settingsClientCertificates)
        {
            if (settingsClientCertificates is null || settingsClientCertificates.Count <= 0)
                return null;
            var collection = new X509Certificate2Collection();
            foreach (var cert in settingsClientCertificates.Select(kv => GetCachedCertificate(kv.Key, kv.Value)))
                collection.Add(cert);
            return collection;
        }

        public static X509Certificate2 GetCachedCertificate(string thumb, StoreLocation location)
        {
            return GetCertificate(thumb, location)/*Certificates.GetOrAdd(thumb, _ => GetCertificate(thumb, location))*/;
        }

        public static X509Certificate2 GetCertificate(string thumb, StoreLocation storeLocation)
        {
            thumb = Regex.Replace(thumb, @"[^\da-fA-F]", string.Empty).ToUpper();
            var storeName = StoreName.My;
            var store = new X509Store(storeName, storeLocation);
            try
            {
                store.Open(OpenFlags.ReadOnly);

                X509Certificate2Collection collection =
                    store.Certificates.Find(X509FindType.FindByThumbprint, thumb, false);

                return collection.OfType<X509Certificate2>().FirstOrDefault() ?? throw new Exception($"Not found certificate for {nameof(thumb)}='{thumb}' in {nameof(storeName)}={storeName} and {nameof(storeLocation)}={storeLocation}");
            }
            finally
            {
                store.Close();
            }
        }

        //private static readonly ConcurrentDictionary<string, X509Certificate2> Certificates = new ConcurrentDictionary<string, X509Certificate2>();
    }
}
