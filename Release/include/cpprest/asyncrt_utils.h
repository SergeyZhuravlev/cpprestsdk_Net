/***
* Copyright (C) Microsoft. All rights reserved.
* Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
*
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* Various common utilities.
*
* For the latest on this and related APIs, please see: https://github.com/Microsoft/cpprestsdk
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <system_error>
#include <random>
/*#include <locale.h>

#include "pplx/pplxtasks.h"*/
#include "cpprest/details/basic_types.h"
#include "cpprest/CppRestNativeExport.h"

#if !defined(_WIN32) || (_MSC_VER >= 1700)
#include <chrono>
#endif

#ifndef _WIN32
#include <boost/algorithm/string.hpp>
#if !defined(ANDROID) && !defined(__ANDROID__) && !defined(__GLIBC__) // CodePlex 269
/* Systems using glibc: xlocale.h has been removed from glibc 2.26
   The above include of locale.h is sufficient
   Further details: https://sourceware.org/git/?p=glibc.git;a=commit;h=f0be25b6336db7492e47d2e8e72eb8af53b5506d */
#include <xlocale.h>
#endif
#endif

/// Various utilities for string conversions and date and time manipulation.
#include "warnings\dll_export_warnings_disable.h"
namespace utility
{

// Left over from VS2010 support, remains to avoid breaking.
typedef std::chrono::seconds seconds;

/// Functions for converting to/from std::chrono::seconds to xml string.
namespace timespan
{
    /// <summary>
    /// Converts a timespan/interval in seconds to xml duration string as specified by
    /// http://www.w3.org/TR/xmlschema-2/#duration
    /// </summary>
    CPPRESTNATIVE_API utility::string_t __cdecl seconds_to_xml_duration(utility::seconds numSecs);

    /// <summary>
    /// Converts an xml duration to timespan/interval in seconds
    /// http://www.w3.org/TR/xmlschema-2/#duration
    /// </summary>
    CPPRESTNATIVE_API utility::seconds __cdecl xml_duration_to_seconds(const utility::string_t &timespanString);
}

/// Functions for Unicode string conversions.
namespace conversions
{
    /// <summary>
    /// Converts a UTF-16 string to a UTF-8 string.
    /// </summary>
    /// <param name="w">A two byte character UTF-16 string.</param>
    /// <returns>A single byte character UTF-8 string.</returns>
    CPPRESTNATIVE_API std::string __cdecl utf16_to_utf8(const utf16string &w);

    /// <summary>
    /// Converts a UTF-8 string to a UTF-16
    /// </summary>
    /// <param name="s">A single byte character UTF-8 string.</param>
    /// <returns>A two byte character UTF-16 string.</returns>
    CPPRESTNATIVE_API utf16string __cdecl utf8_to_utf16(const std::string &s);

    /// <summary>
    /// Converts a ASCII (us-ascii) string to a UTF-16 string.
    /// </summary>
    /// <param name="s">A single byte character us-ascii string.</param>
    /// <returns>A two byte character UTF-16 string.</returns>
    CPPRESTNATIVE_API utf16string __cdecl usascii_to_utf16(const std::string &s);

    /// <summary>
    /// Converts a Latin1 (iso-8859-1) string to a UTF-16 string.
    /// </summary>
    /// <param name="s">A single byte character UTF-8 string.</param>
    /// <returns>A two byte character UTF-16 string.</returns>
    CPPRESTNATIVE_API utf16string __cdecl latin1_to_utf16(const std::string &s);

    /// <summary>
    /// Converts a Latin1 (iso-8859-1) string to a UTF-8 string.
    /// </summary>
    /// <param name="s">A single byte character UTF-8 string.</param>
    /// <returns>A single byte character UTF-8 string.</returns>
    CPPRESTNATIVE_API utf8string __cdecl latin1_to_utf8(const std::string &s);

    /// <summary>
    /// Converts to a platform dependent Unicode string type.
    /// </summary>
    /// <param name="s">A single byte character UTF-8 string.</param>
    /// <returns>A platform dependent string type.</returns>
#ifdef _UTF16_STRINGS
    CPPRESTNATIVE_API utility::string_t __cdecl to_string_t(std::string &&s);
#else
    inline utility::string_t&& to_string_t(std::string &&s) { return std::move(s); }
#endif

    /// <summary>
    /// Converts to a platform dependent Unicode string type.
    /// </summary>
    /// <param name="s">A two byte character UTF-16 string.</param>
    /// <returns>A platform dependent string type.</returns>
#ifdef _UTF16_STRINGS
    inline utility::string_t&& to_string_t(utf16string &&s) { return std::move(s); }
#else
    CPPRESTNATIVE_API utility::string_t __cdecl to_string_t(utf16string &&s);
#endif
    /// <summary>
    /// Converts to a platform dependent Unicode string type.
    /// </summary>
    /// <param name="s">A single byte character UTF-8 string.</param>
    /// <returns>A platform dependent string type.</returns>
#ifdef _UTF16_STRINGS
    CPPRESTNATIVE_API utility::string_t __cdecl to_string_t(const std::string &s);
#else
    inline const utility::string_t& to_string_t(const std::string &s) { return s; }
#endif

    /// <summary>
    /// Converts to a platform dependent Unicode string type.
    /// </summary>
    /// <param name="s">A two byte character UTF-16 string.</param>
    /// <returns>A platform dependent string type.</returns>
#ifdef _UTF16_STRINGS
    inline const utility::string_t& to_string_t(const utf16string &s) { return s; }
#else
    CPPRESTNATIVE_API utility::string_t __cdecl to_string_t(const utf16string &s);
#endif

    /// <summary>
    /// Converts to a UTF-16 from string.
    /// </summary>
    /// <param name="value">A single byte character UTF-8 string.</param>
    /// <returns>A two byte character UTF-16 string.</returns>
    CPPRESTNATIVE_API utf16string __cdecl to_utf16string(const std::string &value);

    /// <summary>
    /// Converts to a UTF-16 from string.
    /// </summary>
    /// <param name="value">A two byte character UTF-16 string.</param>
    /// <returns>A two byte character UTF-16 string.</returns>
    inline const utf16string& to_utf16string(const utf16string& value)
    {
        return value;
    }
    /// <summary>
    /// Converts to a UTF-16 from string.
    /// </summary>
    /// <param name="value">A two byte character UTF-16 string.</param>
    /// <returns>A two byte character UTF-16 string.</returns>
    inline utf16string&& to_utf16string(utf16string&& value)
    {
        return std::move(value);
    }

    /// <summary>
    /// Converts to a UTF-8 string.
    /// </summary>
    /// <param name="value">A single byte character UTF-8 string.</param>
    /// <returns>A single byte character UTF-8 string.</returns>
    inline std::string&& to_utf8string(std::string&& value) { return std::move(value); }

    /// <summary>
    /// Converts to a UTF-8 string.
    /// </summary>
    /// <param name="value">A single byte character UTF-8 string.</param>
    /// <returns>A single byte character UTF-8 string.</returns>
    inline const std::string& to_utf8string(const std::string& value) { return value; }

    /// <summary>
    /// Converts to a UTF-8 string.
    /// </summary>
    /// <param name="value">A two byte character UTF-16 string.</param>
    /// <returns>A single byte character UTF-8 string.</returns>
    CPPRESTNATIVE_API std::string __cdecl to_utf8string(const utf16string &value);

    /// <summary>
    /// Encode the given byte array into a base64 string
    /// </summary>
    CPPRESTNATIVE_API utility::string_t __cdecl to_base64(const std::vector<unsigned char>& data);

    /// <summary>
    /// Encode the given 8-byte integer into a base64 string
    /// </summary>
    CPPRESTNATIVE_API utility::string_t __cdecl to_base64(uint64_t data);

    /// <summary>
    /// Decode the given base64 string to a byte array
    /// </summary>
    CPPRESTNATIVE_API std::vector<unsigned char> __cdecl from_base64(const utility::string_t& str);

	/// <summary>
	/// Decode the given base64 string to a byte array (for internal usage only)
	/// </summary>
	CPPRESTNATIVE_API utility::string_t _to_base64(const unsigned char *ptr, size_t size);

    template <typename Source>
    CASABLANCA_DEPRECATED("All locale-sensitive APIs will be removed in a future update. Use stringstreams directly if locale support is required.")
    utility::string_t print_string(const Source &val, const std::locale& loc = std::locale())
    {
        utility::ostringstream_t oss;
        oss.imbue(loc);
        oss << val;
        if (oss.bad())
        {
            throw std::bad_cast();
        }
        return oss.str();
    }

    CASABLANCA_DEPRECATED("All locale-sensitive APIs will be removed in a future update. Use stringstreams directly if locale support is required.")
    inline utility::string_t print_string(const utility::string_t &val)
    {
        return val;
    }

    namespace details
    {

#if defined(__ANDROID__)
        template<class T>
        inline std::string to_string(const T& t)
        {
            std::ostringstream os;
            os.imbue(std::locale::classic());
            os << t;
            return os.str();
        }
#endif

        template<class T>
        inline utility::string_t to_string_t(T&& t)
        {
#ifdef _UTF16_STRINGS
            using std::to_wstring;
            return to_wstring(std::forward<T>(t));
#else
#if !defined(__ANDROID__)
            using std::to_string;
#endif
            return to_string(std::forward<T>(t));
#endif
        }

        template <typename Source>
        utility::string_t print_string(const Source &val)
        {
            utility::ostringstream_t oss;
            oss.imbue(std::locale::classic());
            oss << val;
            if (oss.bad())
            {
                throw std::bad_cast();
            }
            return oss.str();
        }

        inline const utility::string_t& print_string(const utility::string_t &val)
        {
            return val;
        }

        template<typename Source>
        utf8string print_utf8string(const Source& val)
        {
            return conversions::to_utf8string(print_string(val));
        }
        inline const utf8string& print_utf8string(const utf8string& val)
        {
            return val;
        }

        template <typename Target>
        Target scan_string(const utility::string_t &str)
        {
            Target t;
            utility::istringstream_t iss(str);
            iss.imbue(std::locale::classic());
            iss >> t;
            if (iss.bad())
            {
                throw std::bad_cast();
            }
            return t;
        }

        inline const utility::string_t& scan_string(const utility::string_t &str)
        {
            return str;
        }
    }

    template <typename Target>
    CASABLANCA_DEPRECATED("All locale-sensitive APIs will be removed in a future update. Use stringstreams directly if locale support is required.")
    Target scan_string(const utility::string_t &str, const std::locale &loc = std::locale())
    {
        Target t;
        utility::istringstream_t iss(str);
        iss.imbue(loc);
        iss >> t;
        if (iss.bad())
        {
            throw std::bad_cast();
        }
        return t;
    }

    CASABLANCA_DEPRECATED("All locale-sensitive APIs will be removed in a future update. Use stringstreams directly if locale support is required.")
    inline utility::string_t scan_string(const utility::string_t &str)
    {
        return str;
    }
}

namespace details
{
    /// <summary>
    /// Cross platform RAII container for setting thread local locale.
    /// </summary>
    class scoped_c_thread_locale
    {
    public:
        CPPRESTNATIVE_API scoped_c_thread_locale();
        CPPRESTNATIVE_API ~scoped_c_thread_locale();

#if !defined(ANDROID) && !defined(__ANDROID__) // CodePlex 269
#ifdef _WIN32
        typedef _locale_t xplat_locale;
#else
        typedef locale_t xplat_locale;
#endif

        static CPPRESTNATIVE_API xplat_locale __cdecl c_locale();
#endif
    private:
#ifdef _WIN32
        std::string m_prevLocale;
        int m_prevThreadSetting;
#elif !(defined(ANDROID) || defined(__ANDROID__))
        locale_t m_prevLocale;
#endif
        scoped_c_thread_locale(const scoped_c_thread_locale &);
        scoped_c_thread_locale & operator=(const scoped_c_thread_locale &);
    };

    /// <summary>
    /// Our own implementation of alpha numeric instead of std::isalnum to avoid
    /// taking global lock for performance reasons.
    /// </summary>
    inline bool __cdecl is_alnum(char ch)
    {
        return (ch >= '0' && ch <= '9')
            || (ch >= 'A' && ch <= 'Z')
            || (ch >= 'a' && ch <= 'z');
    }

    /// <summary>
    /// Simplistic implementation of make_unique. A better implementation would be based on variadic templates
    /// and therefore not be compatible with Dev10.
    /// </summary>
    template <typename _Type>
    std::unique_ptr<_Type> make_unique() {
        return std::unique_ptr<_Type>(new _Type());
    }

    template <typename _Type, typename _Arg1>
    std::unique_ptr<_Type> make_unique(_Arg1&& arg1) {
        return std::unique_ptr<_Type>(new _Type(std::forward<_Arg1>(arg1)));
    }

    template <typename _Type, typename _Arg1, typename _Arg2>
    std::unique_ptr<_Type> make_unique(_Arg1&& arg1, _Arg2&& arg2) {
        return std::unique_ptr<_Type>(new _Type(std::forward<_Arg1>(arg1), std::forward<_Arg2>(arg2)));
    }

    template <typename _Type, typename _Arg1, typename _Arg2, typename _Arg3>
    std::unique_ptr<_Type> make_unique(_Arg1&& arg1, _Arg2&& arg2, _Arg3&& arg3) {
        return std::unique_ptr<_Type>(new _Type(std::forward<_Arg1>(arg1), std::forward<_Arg2>(arg2), std::forward<_Arg3>(arg3)));
    }

    template <typename _Type, typename _Arg1, typename _Arg2, typename _Arg3, typename _Arg4>
    std::unique_ptr<_Type> make_unique(_Arg1&& arg1, _Arg2&& arg2, _Arg3&& arg3, _Arg4&& arg4) {
        return std::unique_ptr<_Type>(new _Type(std::forward<_Arg1>(arg1), std::forward<_Arg2>(arg2), std::forward<_Arg3>(arg3), std::forward<_Arg4>(arg4)));
    }

    /// <summary>
    /// Cross platform utility function for performing case insensitive string comparision.
    /// </summary>
    /// <param name="left">First string to compare.</param>
    /// <param name="right">Second strong to compare.</param>
    /// <returns>true if the strings are equivalent, false otherwise</returns>
    inline bool str_icmp(const utility::string_t &left, const utility::string_t &right)
    {
#ifdef _WIN32
        return _wcsicmp(left.c_str(), right.c_str()) == 0;
#else
        return boost::iequals(left, right);
#endif
    }

#ifdef _WIN32

/// <summary>
/// Category error type for Windows OS errors.
/// </summary>
class windows_category_impl : public std::error_category
{
public:
    virtual const char *name() const CPPREST_NOEXCEPT { return "windows"; }

    virtual std::string message(int errorCode) const CPPREST_NOEXCEPT;

    virtual std::error_condition default_error_condition(int errorCode) const CPPREST_NOEXCEPT;
};

/// <summary>
/// Gets the one global instance of the windows error category.
/// </summary>
/// </returns>An error category instance.</returns>
CPPRESTNATIVE_API const std::error_category & __cdecl windows_category();

#else

/// <summary>
/// Gets the one global instance of the linux error category.
/// </summary>
/// </returns>An error category instance.</returns>
CPPRESTNATIVE_API const std::error_category & __cdecl linux_category();

#endif

/// <summary>
/// Gets the one global instance of the current platform's error category.
/// <summary>
CPPRESTNATIVE_API const std::error_category & __cdecl platform_category();

/// <summary>
/// Creates an instance of std::system_error from a OS error code.
/// </summary>
inline std::system_error __cdecl create_system_error(unsigned long errorCode)
{
    std::error_code code((int)errorCode, platform_category());
    return std::system_error(code, code.message());
}

/// <summary>
/// Creates a std::error_code from a OS error code.
/// </summary>
inline std::error_code __cdecl create_error_code(unsigned long errorCode)
{
    return std::error_code((int)errorCode, platform_category());
}

/// <summary>
/// Creates the corresponding error message from a OS error code.
/// </summary>
inline utility::string_t __cdecl create_error_message(unsigned long errorCode)
{
    return utility::conversions::to_string_t(create_error_code(errorCode).message());
}

}

class datetime
{
public:
    typedef uint64_t interval_type;

    /// <summary>
    /// Defines the supported date and time string formats.
    /// </summary>
    enum date_format { RFC_1123, ISO_8601 };

    /// <summary>
    /// Returns the current UTC time.
    /// </summary>
    static CPPRESTNATIVE_API datetime __cdecl utc_now();

    /// <summary>
    /// An invalid UTC timestamp value.
    /// </summary>
    enum:interval_type { utc_timestamp_invalid = static_cast<interval_type>(-1) };

    /// <summary>
    /// Returns seconds since Unix/POSIX time epoch at 01-01-1970 00:00:00.
    /// If time is before epoch, utc_timestamp_invalid is returned.
    /// </summary>
    static interval_type utc_timestamp()
    {
        const auto seconds = utc_now().to_interval() / _secondTicks;
        if (seconds >= 11644473600LL)
        {
            return seconds - 11644473600LL;
        }
        else
        {
            return utc_timestamp_invalid;
        }
    }

    datetime() : m_interval(0)
    {
    }

    /// <summary>
    /// Creates <c>datetime</c> from a string representing time in UTC in RFC 1123 format.
    /// </summary>
    /// <returns>Returns a <c>datetime</c> of zero if not successful.</returns>
    static CPPRESTNATIVE_API datetime __cdecl from_string(const utility::string_t& timestring, date_format format = RFC_1123);

    /// <summary>
    /// Returns a string representation of the <c>datetime</c>.
    /// </summary>
    CPPRESTNATIVE_API utility::string_t to_string(date_format format = RFC_1123) const;

    /// <summary>
    /// Returns the integral time value.
    /// </summary>
    interval_type to_interval() const
    {
        return m_interval;
    }

    datetime operator- (interval_type value) const
    {
        return datetime(m_interval - value);
    }

    datetime operator+ (interval_type value) const
    {
        return datetime(m_interval + value);
    }

    bool operator== (datetime dt) const
    {
        return m_interval == dt.m_interval;
    }

    bool operator!= (const datetime& dt) const
    {
        return !(*this == dt);
    }

    static interval_type from_milliseconds(unsigned int milliseconds)
    {
        return milliseconds*_msTicks;
    }

    static interval_type from_seconds(unsigned int seconds)
    {
        return seconds*_secondTicks;
    }

    static interval_type from_minutes(unsigned int minutes)
    {
        return minutes*_minuteTicks;
    }

    static interval_type from_hours(unsigned int hours)
    {
        return hours*_hourTicks;
    }

    static interval_type from_days(unsigned int days)
    {
        return days*_dayTicks;
    }

    bool is_initialized() const
    {
        return m_interval != 0;
    }

private:

    friend int operator- (datetime t1, datetime t2);

    static const interval_type _msTicks = static_cast<interval_type>(10000);
    static const interval_type _secondTicks = 1000*_msTicks;
    static const interval_type _minuteTicks = 60*_secondTicks;
    static const interval_type _hourTicks   = 60*60*_secondTicks;
    static const interval_type _dayTicks    = 24*60*60*_secondTicks;


#ifdef _WIN32
    // void* to avoid pulling in windows.h
    static CPPRESTNATIVE_API bool __cdecl system_type_to_datetime(/*SYSTEMTIME*/ void* psysTime, uint64_t seconds, datetime * pdt);
#else
    static datetime timeval_to_datetime(const timeval &time);
#endif

    // Private constructor. Use static methods to create an instance.
    datetime(interval_type interval) : m_interval(interval)
    {
    }

    // Storing as hundreds of nanoseconds 10e-7, i.e. 1 here equals 100ns.
    interval_type m_interval;
};

#ifndef _WIN32

// temporary workaround for the fact that
// utf16char is not fully supported in GCC
class cmp
{
public:

    static int icmp(std::string left, std::string right)
    {
        size_t i;
        for (i = 0; i < left.size(); ++i)
        {
            if (i == right.size()) return 1;

            auto l = cmp::tolower(left[i]);
            auto r = cmp::tolower(right[i]);
            if (l > r) return 1;
            if (l < r) return -1;
        }
        if (i < right.size()) return -1;
        return 0;
    }

private:
    static char tolower(char c)
    {
        if (c >= 'A' && c <= 'Z')
            return static_cast<char>(c - 'A' + 'a');
        return c;
    }
};

#endif

inline int operator- (datetime t1, datetime t2)
{
    auto diff = (t1.m_interval - t2.m_interval);

    // Round it down to seconds
    diff /= 10 * 1000 * 1000;

    return static_cast<int>(diff);
}

/// <summary>
/// Nonce string generator class.
/// </summary>
class nonce_generator
{
public:

    /// <summary>
    /// Define default nonce length.
    /// </summary>
    enum { default_length = 32 };

    /// <summary>
    /// Nonce generator constructor.
    /// </summary>
    /// <param name="length">Length of the generated nonce string.</param>
    nonce_generator(int length=default_length) :
        m_random(static_cast<unsigned int>(utility::datetime::utc_timestamp())),
        m_length(length)
    {}

    /// <summary>
    /// Generate a nonce string containing random alphanumeric characters (A-Za-z0-9).
    /// Length of the generated string is set by length().
    /// </summary>
    /// <returns>The generated nonce string.</returns>
    CPPRESTNATIVE_API utility::string_t generate();

    /// <summary>
    /// Get length of generated nonce string.
    /// </summary>
    /// <returns>Nonce string length.</returns>
    int length() const { return m_length; }

    /// <summary>
    /// Set length of the generated nonce string.
    /// </summary>
    /// <param name="length">Lenght of nonce string.</param>
    void set_length(int length) { m_length = length; }

private:
    static const utility::string_t c_allowed_chars;
    std::mt19937 m_random;
    int m_length;
};

	namespace conversions
	{
		/// Helper functions to convert a series of bytes from a charset to utf-8 or utf-16.
/// These APIs deal with checking for and handling byte order marker (BOM).
		enum endianness
		{
			little_endian,
			big_endian,
			unknown
		};
		CPPRESTNATIVE_API endianness check_byte_order_mark(const utf16string& str);

		CPPRESTNATIVE_API std::string convert_utf16le_to_utf8(utf16string src, bool erase_bom);

		CPPRESTNATIVE_API utility::string_t convert_utf16le_to_string_t(utf16string src, bool erase_bom);

		// Helper function to change endian ness from big endian to little endian
		CPPRESTNATIVE_API utf16string big_endian_to_little_endian(utf16string src, bool erase_bom);

		CPPRESTNATIVE_API std::string convert_utf16be_to_utf8(utf16string src, bool erase_bom);

		CPPRESTNATIVE_API utf16string convert_utf16be_to_utf16le(utf16string src, bool erase_bom);

		CPPRESTNATIVE_API utility::string_t convert_utf16be_to_string_t(utf16string src, bool erase_bom);

		CPPRESTNATIVE_API std::string convert_utf16_to_utf8(utf16string src);

		CPPRESTNATIVE_API utf16string convert_utf16_to_utf16(utf16string src);
		CPPRESTNATIVE_API utility::string_t convert_utf16_to_string_t(utf16string src);

		CPPRESTNATIVE_API std::wstring convert_string_t_to_wstring(utility::string_t src);
	}

} // namespace utility;
#include "warnings\dll_export_warnings_restore.h"