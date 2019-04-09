/***
* Copyright (C) Microsoft. All rights reserved.
* Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
*
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* Implementation Details of the http.h layer of messaging
*
* Functions and types for interoperating with http.h from modern C++
*   This file includes windows definitions and should not be included in a public header
*
* For the latest on this and related APIs, please see: https://github.com/Microsoft/cpprestsdk
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/
#pragma once

#include "..\..\..\include\cpprest\details\basic_types.h"
#include "..\..\..\include\cpprest\CppRestNativeExport.h"

namespace web { namespace http
{

	/// <summary>
	/// Common HTTP methods.
	/// </summary>
	class methods
	{
	public:
#define _METHODS
#define DAT(a,b) CPPRESTNATIVE_API const static method a;
#include "..\..\..\include\cpprest\details\http_constants.dat"
#undef _METHODS
#undef DAT
	};

	/// <summary>
	/// Predefined values for all of the standard HTTP 1.1 response status codes.
	/// </summary>
	class status_codes
	{
	public:
#define _PHRASES
#define DAT(a,b,c) CPPRESTNATIVE_API const static status_code a=b;
#include "..\..\..\include\cpprest\details\http_constants.dat"
#undef _PHRASES
#undef DAT
	};

	/// <summary>
	/// Constants for the HTTP headers mentioned in RFC 2616.
	/// </summary>
	class header_names
	{
	public:
#define _HEADER_NAMES
#define DAT(a,b) CPPRESTNATIVE_API const static utility::string_t a;
#include "..\..\..\include\cpprest\details\http_constants.dat"
#undef _HEADER_NAMES
#undef DAT
	};

	namespace details {

		/// <summary>
		/// Constants for MIME types.
		/// </summary>
		class mime_types
		{
		public:
#define _MIME_TYPES
#define DAT(a,b) CPPRESTNATIVE_API const static utility::string_t a;
#include "..\..\..\include\cpprest\details\http_constants.dat"
#undef _MIME_TYPES
#undef DAT
		};

		/// <summary>
		/// Constants for charset types.
		/// </summary>
		class charset_types
		{
		public:
#define _CHARSET_TYPES
#define DAT(a,b) CPPRESTNATIVE_API const static utility::string_t a;
#include "..\..\..\include\cpprest\details\http_constants.dat"
#undef _CHARSET_TYPES
#undef DAT
		};

	}

namespace details
{

    namespace chunked_encoding
    {
        // Transfer-Encoding: chunked support
        static const size_t additional_encoding_space = 12;
        static const size_t data_offset               = additional_encoding_space-2;

        // Add the data necessary for properly sending data with transfer-encoding: chunked.
        //
        // There are up to 12 additional bytes needed for each chunk:
        //
        // The last chunk requires 5 bytes, and is fixed.
        // All other chunks require up to 8 bytes for the length, and four for the two CRLF
        // delimiters.
        //
        CPPRESTNATIVE_API size_t __cdecl add_chunked_delimiters(_Out_writes_(buffer_size) uint8_t *data, _In_ size_t buffer_size, size_t bytes_read);
    }

    namespace compression
    {
        enum class compression_algorithm : int 
        { 
            deflate = 15,
            gzip = 31,
            invalid = 9999
        };

        using data_buffer = std::vector<uint8_t>;

        class stream_decompressor
        {
        public:

            static compression_algorithm to_compression_algorithm(const utility::string_t& alg)
            {
                if (U("gzip") == alg)
                {
                    return compression_algorithm::gzip;
                }
                else if (U("deflate") == alg)
                {
                    return compression_algorithm::deflate;
                }

                return compression_algorithm::invalid;
            }

            CPPRESTNATIVE_API static bool __cdecl is_supported();

            CPPRESTNATIVE_API stream_decompressor(compression_algorithm alg);

            CPPRESTNATIVE_API data_buffer decompress(const data_buffer& input);

            CPPRESTNATIVE_API data_buffer decompress(const uint8_t* input, size_t input_size);

            CPPRESTNATIVE_API bool has_error() const;

        private:
            class stream_decompressor_impl;
            std::shared_ptr<stream_decompressor_impl> m_pimpl;
        };

        class stream_compressor
        {
        public:

            CPPRESTNATIVE_API static bool __cdecl is_supported();

            CPPRESTNATIVE_API stream_compressor(compression_algorithm alg);

            CPPRESTNATIVE_API data_buffer compress(const data_buffer& input, bool finish);

            CPPRESTNATIVE_API data_buffer compress(const uint8_t* input, size_t input_size, bool finish);

            CPPRESTNATIVE_API bool has_error() const;

        private:
            class stream_compressor_impl;
            std::shared_ptr<stream_compressor_impl> m_pimpl;
        };

    }
}}}
