#pragma once
#include <cpprest\CppRestNativeExport.h>
#include "http_msg_native_base.h"

namespace web::http::details
{
#include "warnings\dll_export_warnings_disable.h"
	class CPPRESTNATIVE_API http_response_base: public http_msg_native_base
	{

	};
#include "warnings\dll_export_warnings_restore.h"
}
