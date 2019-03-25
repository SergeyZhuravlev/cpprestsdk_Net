#pragma once
#include <string>
#include <msclr/marshal_cppstd.h>

namespace
{
	std::string marshal_as_encoding(System::String^ s, System::String ^encoding)
	{
		auto bytes = System::Text::Encoding::GetEncoding(encoding)->GetBytes(s);
		if (s->Equals(System::String::Empty))
			return std::string();
		pin_ptr<unsigned char> p = &bytes[0];
		char *np = (char*)p;
		return std::string(np, bytes->Length);
	}

	System::String^ marshal_as_encoding(const std::string& s, System::String ^encoding)
	{
		auto bytes = gcnew array<unsigned char>(s.size());
		for (size_t i = 0; i < s.size(); ++i)
		{
			bytes[i] = s[i];
		}
		return System::Text::Encoding::GetEncoding(encoding)->GetString(bytes);
	}

	std::string marshal_1251(System::String^ s)
	{
		return marshal_as_encoding(s, "windows-1251");
	}

	System::String^ marshal_1251(const std::string& s)
	{
		return marshal_as_encoding(s, "windows-1251");
	}
}