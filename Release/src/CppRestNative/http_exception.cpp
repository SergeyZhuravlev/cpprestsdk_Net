#include "stdafx.h"
#include "cpprest\http_exception.h"
#include "cpprest\asyncrt_utils.h"

/// <summary>
/// Represents an HTTP error. This class holds an error message and an optional error code.
/// </summary>

/// <summary>
/// Creates an <c>http_exception</c> with just a string message and no error code.
/// </summary>
/// <param name="whatArg">Error message string.</param>
http_exception::http_exception(const utility::string_t &whatArg)
	: m_msg(utility::conversions::to_utf8string(whatArg)) {}

#ifdef _WIN32
/// <summary>
/// Creates an <c>http_exception</c> with just a string message and no error code.
/// </summary>
/// <param name="whatArg">Error message string.</param>
http_exception::http_exception(std::string whatArg) : m_msg(std::move(whatArg)) {}
#endif

/// <summary>
/// Creates an <c>http_exception</c> with from a error code using the current platform error category.
/// The message of the error code will be used as the what() string message.
/// </summary>
/// <param name="errorCode">Error code value.</param>
http_exception::http_exception(int errorCode)
	: m_errorCode(utility::details::create_error_code(errorCode))
{
	m_msg = m_errorCode.message();
}

/// <summary>
/// Creates an <c>http_exception</c> with from a error code using the current platform error category.
/// </summary>
/// <param name="errorCode">Error code value.</param>
/// <param name="whatArg">Message to use in what() string.</param>
http_exception::http_exception(int errorCode, const utility::string_t &whatArg)
	: m_errorCode(utility::details::create_error_code(errorCode)),
	m_msg(utility::conversions::to_utf8string(whatArg))
{}

#ifdef _WIN32
/// <summary>
/// Creates an <c>http_exception</c> with from a error code using the current platform error category.
/// </summary>
/// <param name="errorCode">Error code value.</param>
/// <param name="whatArg">Message to use in what() string.</param>
http_exception::http_exception(int errorCode, std::string whatArg) :
	m_errorCode(utility::details::create_error_code(errorCode)),
	m_msg(std::move(whatArg))
{}
#endif

/// <summary>
/// Creates an <c>http_exception</c> with from a error code and category. The message of the error code will be used
/// as the <c>what</c> string message.
/// </summary>
/// <param name="errorCode">Error code value.</param>
/// <param name="cat">Error category for the code.</param>
http_exception::http_exception(int errorCode, const std::error_category &cat) : m_errorCode(std::error_code(errorCode, cat))
{
	m_msg = m_errorCode.message();
}

/// <summary>
/// Gets a string identifying the cause of the exception.
/// </summary>
/// <returns>A null terminated character string.</returns>
const char* http_exception::what() const
{
	return m_msg.c_str();
}

/// <summary>
/// Retrieves the underlying error code causing this exception.
/// </summary>
/// <returns>A std::error_code.</returns>
const std::error_code & http_exception::error_code() const
{
	return m_errorCode;
}