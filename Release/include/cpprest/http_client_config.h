#pragma once
#include "..\..\include\cpprest\details\web_utilities.h"
#include <list>
#include <vector>
#include <string>

namespace web::http::client
{

	// credentials and web_proxy class has been moved from web::http::client namespace to web namespace.
	// The below using declarations ensure we don't break existing code.
	// Please use the web::credentials and web::web_proxy class going forward.
	using web::credentials;
	using web::web_proxy;

	/// <summary>
/// HTTP client configuration class, used to set the possible configuration options
/// used to create an http_client instance.
/// </summary>
	class http_client_config
	{
	public:
		http_client_config() :
			m_guarantee_order(false),
			m_timeout(std::chrono::seconds(30)),
			m_chunksize(0),
			m_request_compressed(false)
#if !defined(__cplusplus_winrt)
			, m_validate_certificates(true)
#endif
#if !defined(_WIN32) && !defined(__cplusplus_winrt) || defined(CPPREST_FORCE_HTTP_CLIENT_ASIO)
			, m_tlsext_sni_enabled(true)
#endif
#if defined(_WIN32) && !defined(__cplusplus_winrt)
			, m_buffer_request(false)
#endif
		{
		}

#if !defined(CPPREST_TARGET_XP)
		/// <summary>
		/// Get OAuth 1.0 configuration.
		/// </summary>
		/// <returns>Shared pointer to OAuth 1.0 configuration.</returns>
		const std::shared_ptr<oauth1::experimental::oauth1_config> oauth1() const
		{
			return m_oauth1;
		}

		/// <summary>
		/// Set OAuth 1.0 configuration.
		/// </summary>
		/// <param name="config">OAuth 1.0 configuration to set.</param>
		void set_oauth1(oauth1::experimental::oauth1_config config)
		{
			m_oauth1 = std::make_shared<oauth1::experimental::oauth1_config>(std::move(config));
		}
#endif

		/*/// <summary>
		/// Get OAuth 2.0 configuration.
		/// </summary>
		/// <returns>Shared pointer to OAuth 2.0 configuration.</returns>
		const std::shared_ptr<oauth2::experimental::oauth2_config> oauth2() const
		{
			return m_oauth2;
		}

		/// <summary>
		/// Set OAuth 2.0 configuration.
		/// </summary>
		/// <param name="config">OAuth 2.0 configuration to set.</param>
		void set_oauth2(oauth2::experimental::oauth2_config config)
		{
			m_oauth2 = std::make_shared<oauth2::experimental::oauth2_config>(std::move(config));
		}*/

		/// <summary>
		/// Get the web proxy object
		/// </summary>
		/// <returns>A reference to the web proxy object.</returns>
		const web_proxy& proxy() const
		{
			return m_proxy;
		}

		/// <summary>
		/// Set the web proxy object
		/// </summary>
		/// <param name="proxy">A reference to the web proxy object.</param>
		void set_proxy(web_proxy proxy)
		{
			m_proxy = std::move(proxy);
		}

		/// <summary>
		/// Get the client credentials
		/// </summary>
		/// <returns>A reference to the client credentials.</returns>
		const http::client::credentials& credentials() const
		{
			return m_credentials;
		}

		/// <summary>
		/// Set the client credentials
		/// </summary>
		/// <param name="cred">A reference to the client credentials.</param>
		void set_credentials(const http::client::credentials& cred)
		{
			m_credentials = cred;
		}

		/// <summary>
		/// Get the 'guarantee order' property
		/// </summary>
		/// <returns>The value of the property.</returns>
		bool guarantee_order() const
		{
			return m_guarantee_order;
		}

		/// <summary>
		/// Set the 'guarantee order' property
		/// </summary>
		/// <param name="guarantee_order">The value of the property.</param>
		CASABLANCA_DEPRECATED("Confusing API will be removed in future releases. If you need to order HTTP requests use task continuations.")
			void set_guarantee_order(bool guarantee_order)
		{
			m_guarantee_order = guarantee_order;
		}

		/// <summary>
		/// Get the timeout
		/// </summary>
		/// <returns>The timeout (in seconds) used for each send and receive operation on the client.</returns>
		utility::seconds timeout() const
		{
			return std::chrono::duration_cast<utility::seconds>(m_timeout);
		}

		/// <summary>
		/// Get the timeout
		/// </summary>
		/// <returns>The timeout (in whatever duration) used for each send and receive operation on the client.</returns>
		template <class T>
		T timeout() const
		{
			return std::chrono::duration_cast<T>(m_timeout);
		}
		/// <summary>
		/// Set the timeout
		/// </summary>
		/// <param name="timeout">The timeout (duration from microseconds range and up) used for each send and receive operation on the client.</param>
		template <class T>
		void set_timeout(const T &timeout)
		{
			m_timeout = std::chrono::duration_cast<std::chrono::microseconds>(timeout);
		}

		/// <summary>
		/// Get the client chunk size.
		/// </summary>
		/// <returns>The internal buffer size used by the http client when sending and receiving data from the network.</returns>
		size_t chunksize() const
		{
			return m_chunksize == 0 ? 64 * 1024 : m_chunksize;
		}

		/// <summary>
		/// Sets the client chunk size.
		/// </summary>
		/// <param name="size">The internal buffer size used by the http client when sending and receiving data from the network.</param>
		/// <remarks>This is a hint -- an implementation may disregard the setting and use some other chunk size.</remarks>
		void set_chunksize(size_t size)
		{
			m_chunksize = size;
		}

		/// <summary>
		/// Returns true if the default chunk size is in use.
		/// <remarks>If true, implementations are allowed to choose whatever size is best.</remarks>
		/// </summary>
		/// <returns>True if default, false if set by user.</returns>
		bool is_default_chunksize() const
		{
			return m_chunksize == 0;
		}

		/// <summary>
		/// Checks if requesting a compressed response is turned on, the default is off.
		/// </summary>
		/// <returns>True if compressed response is enabled, false otherwise</returns>
		bool request_compressed_response() const
		{
			return m_request_compressed;
		}

		/// <summary>
		/// Request that the server responds with a compressed body.
		/// If true, in cases where the server does not support compression, this will have no effect.
		/// The response body is internally decompressed before the consumer receives the data.
		/// </summary>
		/// <param name="request_compressed">True to turn on response body compression, false otherwise.</param>
		/// <remarks>Please note there is a performance cost due to copying the request data. Currently only supported on Windows and OSX.</remarks>
		void set_request_compressed_response(bool request_compressed)
		{
			m_request_compressed = request_compressed;
		}

		typedef std::list<std::pair<std::string, bool>> certificates_t;

		void add_client_certificate(const std::string& thumb, bool isUserStorage)
		{
			m_client_certificates.push_back({ thumb, isUserStorage });
		}

		const certificates_t& get_client_certificates() const
		{
			return m_client_certificates;
		}

#if !defined(__cplusplus_winrt)
		/// <summary>
		/// Gets the server certificate validation property.
		/// </summary>
		/// <returns>True if certificates are to be verified, false otherwise.</returns>
		bool validate_certificates() const
		{
			return m_validate_certificates;
		}

		/// <summary>
		/// Sets the server certificate validation property.
		/// </summary>
		/// <param name="validate_certs">False to turn ignore all server certificate validation errors, true otherwise.</param>
		/// <remarks>Note ignoring certificate errors can be dangerous and should be done with caution.</remarks>
		void set_validate_certificates(bool validate_certs)
		{
			m_validate_certificates = validate_certs;
		}
#endif

#if defined(_WIN32) && !defined(__cplusplus_winrt)
		/// <summary>
		/// Checks if request data buffering is turned on, the default is off.
		/// </summary>
		/// <returns>True if buffering is enabled, false otherwise</returns>
		bool buffer_request() const
		{
			return m_buffer_request;
		}

		/// <summary>
		/// Sets the request buffering property.
		/// If true, in cases where the request body/stream doesn't support seeking the request data will be buffered.
		/// This can help in situations where an authentication challenge might be expected.
		/// </summary>
		/// <param name="buffer_request">True to turn on buffer, false otherwise.</param>
		/// <remarks>Please note there is a performance cost due to copying the request data.</remarks>
		void set_buffer_request(bool buffer_request)
		{
			m_buffer_request = buffer_request;
		}
#endif
		/*
		/// <summary>
		/// Sets a callback to enable custom setting of platform specific options.
		/// </summary>
		/// <remarks>
		/// The native_handle is the following type depending on the underlying platform:
		///     Windows Desktop, WinHTTP - HINTERNET (session)
		/// </remarks>
		/// <param name="callback">A user callback allowing for customization of the session</param>
		void set_nativesessionhandle_options(const std::function<void(native_handle)> &callback)
		{
			m_set_user_nativesessionhandle_options = callback;
		}

		/// <summary>
		/// Invokes a user's callback to allow for customization of the session.
		/// </summary>
		/// <remarks>Internal Use Only</remarks>
		/// <param name="handle">A internal implementation handle.</param>
		void _invoke_nativesessionhandle_options(native_handle handle) const
		{
			if (m_set_user_nativesessionhandle_options)
				m_set_user_nativesessionhandle_options(handle);
		}

		/// <summary>
		/// Sets a callback to enable custom setting of platform specific options.
		/// </summary>
		/// <remarks>
		/// The native_handle is the following type depending on the underlying platform:
		///     Windows Desktop, WinHTTP - HINTERNET
		///     Windows Runtime, WinRT - IXMLHTTPRequest2 *
		///     All other platforms, Boost.Asio:
		///         https - boost::asio::ssl::stream<boost::asio::ip::tcp::socket &> *
		///         http - boost::asio::ip::tcp::socket *
		/// </remarks>
		/// <param name="callback">A user callback allowing for customization of the request</param>
		void set_nativehandle_options(const std::function<void(native_handle)> &callback)
		{
			m_set_user_nativehandle_options = callback;
		}

		/// <summary>
		/// Invokes a user's callback to allow for customization of the request.
		/// </summary>
		/// <param name="handle">A internal implementation handle.</param>
		void invoke_nativehandle_options(native_handle handle) const
		{
			if (m_set_user_nativehandle_options)
				m_set_user_nativehandle_options(handle);
		}*/

#if !defined(_WIN32) && !defined(__cplusplus_winrt) || defined(CPPREST_FORCE_HTTP_CLIENT_ASIO)
		/// <summary>
		/// Sets a callback to enable custom setting of the ssl context, at construction time.
		/// </summary>
		/// <param name="callback">A user callback allowing for customization of the ssl context at construction time.</param>
		void set_ssl_context_callback(const std::function<void(boost::asio::ssl::context&)>& callback)
		{
			m_ssl_context_callback = callback;
		}

		/// <summary>
		/// Gets the user's callback to allow for customization of the ssl context.
		/// </summary>
		const std::function<void(boost::asio::ssl::context&)>& get_ssl_context_callback() const
		{
			return m_ssl_context_callback;
		}

		/// <summary>
		/// Gets the TLS extension server name indication (SNI) status.
		/// </summary>
		/// <returns>True if TLS server name indication is enabled, false otherwise.</returns>
		bool is_tlsext_sni_enabled() const
		{
			return m_tlsext_sni_enabled;
		}

		/// <summary>
		/// Sets the TLS extension server name indication (SNI) status.
		/// </summary>
		/// <param name="tlsext_sni_enabled">False to disable the TLS (ClientHello) extension for server name indication, true otherwise.</param>
		/// <remarks>Note: This setting is enabled by default as it is required in most virtual hosting scenarios.</remarks>
		void set_tlsext_sni_enabled(bool tlsext_sni_enabled)
		{
			m_tlsext_sni_enabled = tlsext_sni_enabled;
		}
#endif

	private:
#if !defined(CPPREST_TARGET_XP)
		std::shared_ptr<oauth1::experimental::oauth1_config> m_oauth1;
#endif

		//std::shared_ptr<oauth2::experimental::oauth2_config> m_oauth2;
		web_proxy m_proxy;
		http::client::credentials m_credentials;
		// Whether or not to guarantee ordering, i.e. only using one underlying TCP connection.
		bool m_guarantee_order;

		std::chrono::microseconds m_timeout;
		size_t m_chunksize;
		bool m_request_compressed;

#if !defined(__cplusplus_winrt)
		// IXmlHttpRequest2 doesn't allow configuration of certificate verification.
		bool m_validate_certificates;
#endif

		/*std::function<void(native_handle)> m_set_user_nativehandle_options;
		std::function<void(native_handle)> m_set_user_nativesessionhandle_options;*/

#if !defined(_WIN32) && !defined(__cplusplus_winrt) || defined(CPPREST_FORCE_HTTP_CLIENT_ASIO)
		std::function<void(boost::asio::ssl::context&)> m_ssl_context_callback;
		bool m_tlsext_sni_enabled;
#endif
#if defined(_WIN32) && !defined(__cplusplus_winrt)
		bool m_buffer_request;
#endif

		certificates_t m_client_certificates;
	};
}