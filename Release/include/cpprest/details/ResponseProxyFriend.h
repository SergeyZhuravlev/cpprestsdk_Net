#pragma once
#include <memory>

namespace web::http::details
{
	class http_response_proxy;

	ref class ResponseProxyFriend
	{
	private:
		std::weak_ptr<http_response_proxy>* _response;

	public:
		virtual ~ResponseProxyFriend();

		explicit ResponseProxyFriend(const std::weak_ptr<http_response_proxy>& response);

		void set_response_stream(System::IO::Stream^ responseStream, System::Int64 contentLength,
		                         System::String^ contentType);
	};
}
