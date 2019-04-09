#pragma once
#include "..\..\include\cpprest\CppRestNativeExport.h"
#include "..\..\include\cpprest\details\basic_types.h"

namespace web::http::details
{
	class HttpCppCliBridge;

	/// <summary>
	/// Base class for HTTP messages.
	/// This class is to store common functionality so it isn't duplicated on
	/// both the request and response side.
	/// </summary>
	class CPPRESTNATIVE_API http_msg_native_base
	{
	public:
		std::shared_ptr<HttpCppCliBridge>& _get_httpCppCliBridge();
		const std::shared_ptr<HttpCppCliBridge>& _get_httpCppCliBridge() const;
		virtual ~http_msg_native_base() = default;

	private:
		std::shared_ptr<HttpCppCliBridge> _httpCppCliBridge;
	};
}
