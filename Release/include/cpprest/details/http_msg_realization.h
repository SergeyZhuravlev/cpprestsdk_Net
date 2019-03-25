#pragma once

/*namespace web::http
{*/
template<class Base>
	http_msg_base<Base>::http_msg_base()
		: m_headers(),
		m_default_outstream(false)
	{
	}

	template<class Base>
	void http_msg_base<Base>::_prepare_to_receive_data()
	{
		// See if the user specified an outstream
		if (!outstream())
		{
			// The user did not specify an outstream.
			// We will create one...
			concurrency::streams::producer_consumer_buffer<uint8_t> buf;
			set_outstream(buf.create_ostream(), true);

			// Since we are creating the streambuffer, set the input stream
			// so that the user can retrieve the data.
			set_instream(buf.create_istream());
		}

		// If the user did specify an outstream we leave the instream
		// as invalid. It is assumed that user either has a read head
		// to the out streambuffer or the data is streamed into a container
		// or media (like file) that the user can read from...
	}

	template<class Base>
	size_t http_msg_base<Base>::_get_content_length()
	{
		// An invalid response_stream indicates that there is no body
		if ((bool)instream())
		{
			size_t content_length = 0;
			utility::string_t transfer_encoding;

			bool has_cnt_length = headers().match(header_names::content_length, content_length);
			bool has_xfr_encode = headers().match(header_names::transfer_encoding, transfer_encoding);

			if (has_xfr_encode)
			{
				return (std::numeric_limits<size_t>::max)();
			}

			if (has_cnt_length)
			{
				return content_length;
			}

			// Neither is set. Assume transfer-encoding for now (until we have the ability to determine
			// the length of the stream).
			headers().add(header_names::transfer_encoding, _XPLATSTR("chunked"));
			return (std::numeric_limits<size_t>::max)();
		}

		return 0;
	}

	template<class Base>
	utility::string_t /*details::*/http_msg_base<Base>::parse_and_check_content_type(bool ignore_content_type, const std::function<bool(const utility::string_t &)> &check_content_type)
	{
		if (!instream())
		{
			throw http_exception(details::stream_was_set_explicitly);
		}

		utility::string_t content, charset = charset_types::utf8;
		if (!ignore_content_type)
		{
			details::parse_content_type_and_charset(headers().content_type(), content, charset);

			// If no Content-Type or empty body then just return an empty string.
			if (content.empty() || instream().streambuf().in_avail() == 0)
			{
				return utility::string_t();
			}

			if (!check_content_type(content))
			{
				throw http_exception(_XPLATSTR("Incorrect Content-Type: must be textual to extract_string, JSON to extract_json."));
			}
		}
		return charset;
	}

	template<class Base>
	utf8string /*details::*/http_msg_base<Base>::extract_utf8string(bool ignore_content_type)
	{
		const auto &charset = parse_and_check_content_type(ignore_content_type, details::is_content_type_textual);
		if (charset.empty())
		{
			return utf8string();
		}
		auto buf_r = instream().streambuf();

		// Perform the correct character set conversion if one is necessary.
		if (utility::details::str_icmp(charset, charset_types::utf8)
			|| utility::details::str_icmp(charset, charset_types::usascii)
			|| utility::details::str_icmp(charset, charset_types::ascii))
		{
			std::string body;
			body.resize((std::string::size_type)buf_r.in_avail());
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size()).get(); // There is no risk of blocking.
			return body;
		}

		// Latin1
		else if (utility::details::str_icmp(charset, charset_types::latin1))
		{
			std::string body;
			body.resize((std::string::size_type)buf_r.in_avail());
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size()).get(); // There is no risk of blocking.
			return utility::conversions::latin1_to_utf8(std::move(body));
		}

		// utf-16
		else if (utility::details::str_icmp(charset, charset_types::utf16))
		{
			utf16string body;
			body.resize(buf_r.in_avail() / sizeof(utf16string::value_type));
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size() * sizeof(utf16string::value_type)); // There is no risk of blocking.
			return utility::conversions::convert_utf16_to_utf8(std::move(body));
		}

		// utf-16le
		else if (utility::details::str_icmp(charset, charset_types::utf16le))
		{
			utf16string body;
			body.resize(buf_r.in_avail() / sizeof(utf16string::value_type));
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size() * sizeof(utf16string::value_type)); // There is no risk of blocking.
			return utility::conversions::utf16_to_utf8(std::move(body));
		}

		// utf-16be
		else if (utility::details::str_icmp(charset, charset_types::utf16be))
		{
			utf16string body;
			body.resize(buf_r.in_avail() / sizeof(utf16string::value_type));
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size() * sizeof(utf16string::value_type)); // There is no risk of blocking.
			return utility::conversions::convert_utf16be_to_utf8(std::move(body), false);
		}

		else
		{
			throw http_exception(details::unsupported_charset);
		}
	}

	template<class Base>
	utf16string /*details::*/http_msg_base<Base>::extract_utf16string(bool ignore_content_type)
	{
		const auto &charset = parse_and_check_content_type(ignore_content_type, details::is_content_type_textual);
		if (charset.empty())
		{
			return utf16string();
		}
		auto buf_r = instream().streambuf();

		// Perform the correct character set conversion if one is necessary.
		if (utility::details::str_icmp(charset, charset_types::utf16le))
		{
			utf16string body;
			body.resize(buf_r.in_avail() / sizeof(utf16string::value_type));
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size() * sizeof(utf16string::value_type)); // There is no risk of blocking.
			return body;
		}

		// utf-8, ascii
		else if (utility::details::str_icmp(charset, charset_types::utf8)
			|| utility::details::str_icmp(charset, charset_types::usascii)
			|| utility::details::str_icmp(charset, charset_types::ascii))
		{
			std::string body;
			body.resize((std::string::size_type)buf_r.in_avail());
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size()).get(); // There is no risk of blocking.
			return utility::conversions::utf8_to_utf16(std::move(body));
		}

		// Latin1
		else if (utility::details::str_icmp(charset, charset_types::latin1))
		{
			std::string body;
			body.resize((std::string::size_type)buf_r.in_avail());
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size()).get(); // There is no risk of blocking.
			return utility::conversions::latin1_to_utf16(std::move(body));
		}

		// utf-16
		else if (utility::details::str_icmp(charset, charset_types::utf16))
		{
			utf16string body;
			body.resize(buf_r.in_avail() / sizeof(utf16string::value_type));
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size() * sizeof(utf16string::value_type)); // There is no risk of blocking.
			return utility::conversions::convert_utf16_to_utf16(std::move(body));
		}

		// utf-16be
		else if (utility::details::str_icmp(charset, charset_types::utf16be))
		{
			utf16string body;
			body.resize(buf_r.in_avail() / sizeof(utf16string::value_type));
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size() * sizeof(utf16string::value_type)); // There is no risk of blocking.
			return utility::conversions::convert_utf16be_to_utf16le(std::move(body), false);
		}

		else
		{
			throw http_exception(details::unsupported_charset);
		}
	}

	template<class Base>
	utility::string_t /*details::*/http_msg_base<Base>::extract_string(bool ignore_content_type)
	{
		const auto &charset = parse_and_check_content_type(ignore_content_type, details::is_content_type_textual);
		if (charset.empty())
		{
			return utility::string_t();
		}
		auto buf_r = instream().streambuf();

		// Perform the correct character set conversion if one is necessary.
		if (utility::details::str_icmp(charset, charset_types::usascii)
			|| utility::details::str_icmp(charset, charset_types::ascii))
		{
			std::string body;
			body.resize((std::string::size_type)buf_r.in_avail());
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size()).get(); // There is no risk of blocking.
			return utility::conversions::to_string_t(std::move(body));
		}

		// Latin1
		if (utility::details::str_icmp(charset, charset_types::latin1))
		{
			std::string body;
			body.resize((std::string::size_type)buf_r.in_avail());
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size()).get(); // There is no risk of blocking.
			// Could optimize for linux in the future if a latin1_to_utf8 function was written.
			return utility::conversions::to_string_t(utility::conversions::latin1_to_utf16(std::move(body)));
		}

		// utf-8.
		else if (utility::details::str_icmp(charset, charset_types::utf8))
		{
			std::string body;
			body.resize((std::string::size_type)buf_r.in_avail());
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size()).get(); // There is no risk of blocking.
			return utility::conversions::to_string_t(std::move(body));
		}

		// utf-16.
		else if (utility::details::str_icmp(charset, charset_types::utf16))
		{
			utf16string body;
			body.resize(buf_r.in_avail() / sizeof(utf16string::value_type));
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size() * sizeof(utf16string::value_type)); // There is no risk of blocking.
			return utility::conversions::convert_utf16_to_string_t(std::move(body));
		}

		// utf-16le
		else if (utility::details::str_icmp(charset, charset_types::utf16le))
		{
			utf16string body;
			body.resize(buf_r.in_avail() / sizeof(utf16string::value_type));
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size() * sizeof(utf16string::value_type)); // There is no risk of blocking.
			return utility::conversions::convert_utf16le_to_string_t(std::move(body), false);
		}

		// utf-16be
		else if (utility::details::str_icmp(charset, charset_types::utf16be))
		{
			utf16string body;
			body.resize(buf_r.in_avail() / sizeof(utf16string::value_type));
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size() * sizeof(utf16string::value_type)); // There is no risk of blocking.
			return utility::conversions::convert_utf16be_to_string_t(std::move(body), false);
		}

		else
		{
			throw http_exception(details::unsupported_charset);
		}
	}

	template<class Base>
	json::value /*details::*/http_msg_base<Base>::_extract_json(bool ignore_content_type)
	{
		const auto &charset = parse_and_check_content_type(ignore_content_type, details::is_content_type_json);
		if (charset.empty())
		{
			return json::value();
		}
		auto buf_r = instream().streambuf();

		// Latin1
		if (utility::details::str_icmp(charset, charset_types::latin1))
		{
			std::string body;
			body.resize(buf_r.in_avail());
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size()).get(); // There is no risk of blocking.
			// On Linux could optimize in the future if a latin1_to_utf8 function is written.
			return json::value::parse(utility::conversions::to_string_t(utility::conversions::latin1_to_utf16(std::move(body))));
		}

		// utf-8, usascii and ascii
		else if (utility::details::str_icmp(charset, charset_types::utf8)
			|| utility::details::str_icmp(charset, charset_types::usascii)
			|| utility::details::str_icmp(charset, charset_types::ascii))
		{
			std::string body;
			body.resize(buf_r.in_avail());
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size()).get(); // There is no risk of blocking.
			return json::value::parse(utility::conversions::to_string_t(std::move(body)));
		}

		// utf-16.
		else if (utility::details::str_icmp(charset, charset_types::utf16))
		{
			utf16string body;
			body.resize(buf_r.in_avail() / sizeof(utf16string::value_type));
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size() * sizeof(utf16string::value_type)); // There is no risk of blocking.
			return json::value::parse(utility::conversions::convert_utf16_to_string_t(std::move(body)));
		}

		// utf-16le
		else if (utility::details::str_icmp(charset, charset_types::utf16le))
		{
			utf16string body;
			body.resize(buf_r.in_avail() / sizeof(utf16string::value_type));
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size() * sizeof(utf16string::value_type)); // There is no risk of blocking.
			return json::value::parse(utility::conversions::convert_utf16le_to_string_t(std::move(body), false));
		}

		// utf-16be
		else if (utility::details::str_icmp(charset, charset_types::utf16be))
		{
			utf16string body;
			body.resize(buf_r.in_avail() / sizeof(utf16string::value_type));
			buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size() * sizeof(utf16string::value_type)); // There is no risk of blocking.
			return json::value::parse(utility::conversions::convert_utf16be_to_string_t(std::move(body), false));
		}

		else
		{
			throw http_exception(details::unsupported_charset);
		}
	}

	template<class Base>
	std::vector<uint8_t> /*details::*/http_msg_base<Base>::_extract_vector()
	{
		if (!instream())
		{
			throw http_exception(details::stream_was_set_explicitly);
		}

		std::vector<uint8_t> body;
		auto buf_r = instream().streambuf();
		const size_t size = buf_r.in_avail();
		body.resize(size);
		buf_r.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), size).get(); // There is no risk of blocking.

		return body;
	}

	template<class Base>
	utility::string_t /*details::*/http_msg_base<Base>::to_string() const
	{
		return details::http_headers_body_to_string(m_headers, instream());
	}

	namespace
	{
		void set_content_type_if_not_present(http::http_headers &headers, const utility::string_t &content_type)
		{
			utility::string_t temp;
			if (!headers.match(http::header_names::content_type, temp))
			{
				headers.add(http::header_names::content_type, content_type);
			}
		}
	}

	template<class Base>
	void /*details::*/http_msg_base<Base>::set_body(const concurrency::streams::istream &instream, const utf8string &contentType)
	{
		set_content_type_if_not_present(
			headers(),
#ifdef _UTF16_STRINGS
			utility::conversions::utf8_to_utf16(contentType));
#else
			contentType);
#endif
		set_instream(instream);
	}

	template<class Base>
	void /*details::*/http_msg_base<Base>::set_body(const concurrency::streams::istream &instream, const utf16string &contentType)
	{
		set_content_type_if_not_present(
			headers(),
#ifdef _UTF16_STRINGS
			contentType);
#else
			utility::conversions::utf16_to_utf8(contentType));
#endif
		set_instream(instream);
	}

	template<class Base>
	void /*details::*/http_msg_base<Base>::set_body(const concurrency::streams::istream &instream, utility::size64_t contentLength, const utf8string &contentType)
	{
		headers().set_content_length(contentLength);
		set_body(instream, contentType);
		m_data_available.store(contentLength);
	}

	template<class Base>
	void /*details::*/http_msg_base<Base>::set_body(const concurrency::streams::istream &instream, utility::size64_t contentLength, const utf16string &contentType)
	{
		headers().set_content_length(contentLength);
		set_body(instream, contentType);
		m_data_available.store(contentLength);
	}
//}