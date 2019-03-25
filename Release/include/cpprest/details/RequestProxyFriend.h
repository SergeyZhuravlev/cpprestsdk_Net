#pragma once
#include <memory>

namespace web::http::details
{
	class http_request_proxy;

	ref class RequestProxyFriend
	{
	private:
		std::weak_ptr<http_request_proxy>* _request;

	public:
		virtual ~RequestProxyFriend();

		void set_response_holder(CppRestNetImpl::ResponseImpl^ response);

		explicit RequestProxyFriend(const std::weak_ptr<http_request_proxy>& request);

		void _request_stream_writer(System::IO::Stream^ requestStream, System::Nullable<System::Int64> contentLength);

		System::Nullable<System::Int64> _request_stream_length();

		bool _response_explicit_stream_writer(System::IO::Stream^ responseStream, System::Int64 contentLength);
	};
}
