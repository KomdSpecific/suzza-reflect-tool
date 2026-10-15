#pragma once

#include "SuzzaTypes.h"

#include <string>
#include <vector>

namespace suz
{
	struct FieldInfo
	{
		std::string name;							// 变量名
		std::string typeName;						// 类型名(如果类型是别名则显示类型别名)
		bool bIsEditable = false;					// 是否可更改

		/*enum class FieldType
		{
			Bool, Int, Float, Vec3, Vec4, ObjPtr
		} type;*/

		const bool IsValid() const
		{
			return !name.empty() && !typeName.empty();
		}
	};

	struct ClassInfo
	{
		std::string className;
		std::string classPath;
		std::vector<std::string> superClassName;
		std::vector<FieldInfo> fields;

		const bool IsValid() const
		{
			return !className.empty() && !classPath.empty();
		}
	};
}