#pragma once
namespace cpprestsdk
{
	namespace details
	{
		enum class TriBool: int32_t
		{
			Unknown = 0,
			False = -1,
			True = 1
		};

		namespace 
		{
			bool get_value_or(TriBool source, bool defaultValue)
			{
				switch (source)
				{
				case TriBool::True:
					return true;
				case TriBool::False:
					return false;
				default:
					return defaultValue;
				}
			}
		}
	}
}