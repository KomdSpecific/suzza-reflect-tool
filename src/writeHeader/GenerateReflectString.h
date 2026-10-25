#pragma once

#include <string>

namespace suz
{
	class GenerateReflectString
	{
	public:
		static std::string GenerateClassHeaderString();
		static std::string GenerateClassSourceString();
	};
}