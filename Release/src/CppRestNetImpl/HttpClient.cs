using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Runtime.Remoting.Messaging;
using System.Text;
using System.Threading;
using System.Web;
using CppRest;

namespace CppRestNetImpl
{
    public class ResponseImpl: IDisposable
    {
        public ResponseImpl(HttpWebResponse response, IRequestSettings settings, HttpWebRequest request, CancellationToken token, string netErrorMessage)
        {
            _response = response;
            _request = request;
            _token = token;
            _settings = settings;
            NetErrorMessage = netErrorMessage;
        }

        public void Fill()
        {
            StatusCode = _response.StatusCode;
            StatusDescription = _response.StatusDescription;
            Headers = _response.Headers;
        }

        public Stream ResponseStream => _stream ?? (_stream = GetResponseStream());

        public WebHeaderCollection Headers { get; private set; }
        public HttpStatusCode? StatusCode { get; private set; }
        public string StatusDescription { get; private set; }
        public string NetErrorMessage { get; }

        private Stream GetResponseStream()
        {
            return InvokeAborter.WebRequestInvokeWithCancellation(() =>
            {
                var stream = _response.GetResponseStream();
                if ((!(stream is null)) && stream.CanTimeout)
                {
                    if (!(_settings?.ReadTimeout is null))
                        stream.ReadTimeout = (int)_settings.ReadTimeout.Value.TotalMilliseconds;
                    /*else
                        stream.ReadTimeout = CppRestConsts.DefaultTimeout; */
                }

                return stream;
            }, _token, _request);
        }

        public static bool IsHasResponseStream(HttpWebResponse response)
        {
            return response.Headers.Keys.Cast<string>().Any(key => key.Equals(CppRestConsts.ContentLengthHeader));
        }

        public HttpWebResponse InternalResponse => _response;

        private HttpWebResponse _response;
        private readonly HttpWebRequest _request;
        private readonly IRequestSettings _settings;
        private readonly CancellationToken _token;
        private Stream _stream;

        public void Dispose()
        {
            var response = _response;
            _response = null;
            response?.Close();
        }
    }

    public class RequestImpl
    {
        public RequestImpl(
            Uri uri
            , Action<ResponseImpl> responseHolderSetter
            , IRequestSettings settings = null
            , string method = null
            , WebHeaderCollection headers = null
            , HeaderRangeValue range = null
            /*, long? contentLength = null
            , string contentType = null*/
            , Action<Stream, Int64?> requestStreamWriter = null
            , Func<Int64?> requestStreamLength = null
            , Func<Stream, Int64, bool> responseExplicitStreamWriter = null
            , CancellationToken? token = null)
        {
            _token = token ?? CancellationToken.None;
            _token.ThrowIfCancellationRequested();
            _request = (HttpWebRequest)WebRequest.Create(uri);
            _request.AllowAutoRedirect = true;
            _settings = settings;
            _requestStreamWriter = requestStreamWriter;
            _requestStreamLength = requestStreamLength;
            _responseExplicitStreamWriter = responseExplicitStreamWriter;
            _responseHolderSetter = responseHolderSetter ?? throw new ArgumentNullException(nameof(responseHolderSetter));
            if (!(settings?.TaxcomProxyInfo is null))
                _request.Proxy = ProxyHelper.GetProxyCached(settings?.TaxcomProxyInfo);
            var clientCertificateCollection = CertificateHelper.GetCachedCertificateCollection(settings?.ClientCertificates);
            if (!(clientCertificateCollection is null))
                _request.ClientCertificates = clientCertificateCollection;
            if (!(settings?.ConnectTimeout is null))
                _request.Timeout = (int) settings.ConnectTimeout.Value.TotalMilliseconds;
            /*else
                _request.Timeout = CppRestConsts.DefaultTimeout;*/
            if (!(method is null))
                _request.Method = method;
            /*if (contentLength.HasValue)
                _request.ContentLength = contentLength.Value;
            if (!(contentType is null))
                _request.ContentType = contentType;*/
            if (!(headers is null))
                _request.Headers = headers;
            if (!(settings?.Credentials is null))
                _request.Credentials = settings.Credentials;
            range?.Fill(_request);
            _token.ThrowIfCancellationRequested();
        }

        public HttpWebRequest InternalRequest => _request;
        public Int64? ExplicitResponseStreamContentLength => _explicitResponseStreamLength;

        private void FixEmptyRequestContent()
        {
            var methodsWithForcedContents = new[] { "PUT", "POST" };
            if (!methodsWithForcedContents.Contains(_request.Method.ToUpper()))
                return;
            if (_request.ContentLength <= 0)
            {
                _request.ContentLength = 0;
                if (_request.TransferEncoding is null)
                    _request.SendChunked = true;
                using (_request.GetRequestStream())
                {
                }
            }
        }

        private void FixNotFilledRequestContentLengthIfNeed()
        {
            var methodsWithForcedContents = new[] { "PUT", "POST" };
            if (!methodsWithForcedContents.Contains(_request.Method.ToUpper()))
                return;
            if (_request.ContentLength < 0 && (!(_requestStreamWriter is null)))
            {
                _request.ContentLength = _requestStreamLength?.Invoke() ?? throw new Exception($"Not filled {CppRestConsts.ContentTypeHeader} for {_request.Method} and can't obtain automatically from request stream because of unseekable stream");
                /*if (_request.TransferEncoding is null)
                    _request.SendChunked = true;*/
            }
        }

        private void WriteRequestBody()
        {
            if (_requestStreamWriter is null)
            {
                FixEmptyRequestContent();
                return;
            }
            //FixNotFilledRequestContentLengthIfNeed();
            var outStream = _request.GetRequestStream();
            if (outStream.CanTimeout)
            {
                if (!(_settings?.WriteTimeout is null))
                    outStream.WriteTimeout = (int)_settings.WriteTimeout.Value.TotalMilliseconds;
                /*else
                    outStream.WriteTimeout = CppRestConsts.DefaultTimeout; */
            }
            var length = _request.ContentLength;
            _requestStreamWriter(outStream, length < 0 ? default(Int64?) : length);
        }

        private bool WriteExplicitResponseBody(ResponseImpl responseImpl)
        {
            if (responseImpl is null)
                return false;
            if (_responseExplicitStreamWriter is null)
                return false;
            var response = responseImpl.InternalResponse;
            if (response is null)
                return false;
            if (!ResponseImpl.IsHasResponseStream(response))
                return false;
            var responseContentLength = response.ContentLength;
            if (responseContentLength <= 0)
                return false;
            var responseStream = responseImpl.ResponseStream;
            if (responseStream is null)
                return false;
            _explicitResponseStreamLength = response.ContentLength;
            return _responseExplicitStreamWriter(responseStream, responseContentLength);
        }

        public void SetResponseBody(Action<Stream, long, string> responseStreamSetter)
        {
            var responseImpl = Response;
            if (responseImpl is null)
                return;
            if (responseStreamSetter is null)
                return;
            if (_responseStreamSetterDisabled)
                return;
            var response = responseImpl.InternalResponse;
            if (response is null)
                return;
            if (!ResponseImpl.IsHasResponseStream(response))
                return;
            if (response.ContentLength < 0)
                return;
            var responseStream = responseImpl.ResponseStream;
            if (responseStream is null)
                return;
            responseStreamSetter(responseStream, response.ContentLength, response.ContentType);
        }
        
        public ResponseImpl Response  => _response ?? (_response = GetResponse());

        private ResponseImpl EndingResponse(HttpWebResponse response, string netErrorMessage = null)
        {
            return InvokeAborter.WebRequestInvokeWithCancellation(() =>
            {
                var result = new ResponseImpl(response, _settings, _request, _token, netErrorMessage);
                _responseHolderSetter(result);
                _response = result;
                result.Fill();
                if (WriteExplicitResponseBody(result))
                    _responseStreamSetterDisabled = true;
                return result;
            }, _token, _request);
        }

        private ResponseImpl GetResponse()
        {
            HttpWebResponse response = null;
            try
            {
                InvokeAborter.WebRequestInvokeWithCancellation(() =>
                {
                    WriteRequestBody();
                    response = (HttpWebResponse) _request.GetResponse();
                }, _token, _request);
            }
            catch (WebException ex)
            {
                response = (HttpWebResponse)ex.Response;
                if (response is null)
                    throw;
                return EndingResponse(response, ex.Message);
            }
            return InvokeAborter.WebRequestInvokeWithCancellation(() => EndingResponse(response), _token, _request);
        }

        private readonly IRequestSettings _settings;
        private readonly Action<Stream, Int64?> _requestStreamWriter;
        private readonly Func<Int64?> _requestStreamLength;
        private readonly Func<Stream, Int64, bool> _responseExplicitStreamWriter;
        private readonly Action<ResponseImpl> _responseHolderSetter;
        private readonly HttpWebRequest _request;
        private readonly CancellationToken _token;
        private bool _responseStreamSetterDisabled;
        private ResponseImpl _response;
        private Int64? _explicitResponseStreamLength;
    }

}
