#pragma once
#include <exception>
#include <string>
#include <cpprest/CppRestNativeExport.h>
#include "cpprest\details\basic_types.h"

#include "warnings\dll_export_warnings_disable.h"
class CPPRESTNATIVE_API http_exception : public std::exception
{
public:
	/// <summary>
	/// Creates an <c>http_exception</c> with just a string message and no error code.
	/// </summary>
	/// <param name="whatArg">Error message string.</param>
	http_exception(const utility::string_t &whatArg);

#ifdef _WIN32
	/// <summary>
	/// Creates an <c>http_exception</c> with just a string message and no error code.
	/// </summary>
	/// <param name="whatArg">Error message string.</param>
	http_exception(std::string whatArg);
#endif

	/// <summary>
	/// Creates an <c>http_exception</c> with from a error code using the current platform error category.
	/// The message of the error code will be used as the what() string message.
	/// </summary>
	/// <param name="errorCode">Error code value.</param>
	http_exception(int errorCode);

	/// <summary>
	/// Creates an <c>http_exception</c> with from a error code using the current platform error category.
	/// </summary>
	/// <param name="errorCode">Error code value.</param>
	/// <param name="whatArg">Message to use in what() string.</param>
	http_exception(int errorCode, const utility::string_t &whatArg);

#ifdef _WIN32
	/// <summary>
	/// Creates an <c>http_exception</c> with from a error code using the current platform error category.
	/// </summary>
	/// <param name="errorCode">Error code value.</param>
	/// <param name="whatArg">Message to use in what() string.</param>
	http_exception(int errorCode, std::string whatArg);
#endif

	/// <summary>
	/// Creates an <c>http_exception</c> with from a error code and category. The message of the error code will be used
	/// as the <c>what</c> string message.
	/// </summary>
	/// <param name="errorCode">Error code value.</param>
	/// <param name="cat">Error category for the code.</param>
	http_exception(int errorCode, const std::error_category &cat);

	/// <summary>
	/// Gets a string identifying the cause of the exception.
	/// </summary>
	/// <returns>A null terminated character string.</returns>
	const char* what() const override;

	/// <summary>
	/// Retrieves the underlying error code causing this exception.
	/// </summary>
	/// <returns>A std::error_code.</returns>
	const std::error_code & error_code() const;

private:
	std::error_code m_errorCode;
	std::string m_msg;
};
#include "warnings\dll_export_warnings_restore.h"

namespace web::http
{
	using ::http_exception;
}