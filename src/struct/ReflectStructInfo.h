#pragma once

#include "SuzzaTypes.h"

#include <string>
#include <vector>

namespace suz
{
	enum class ETypeCategory : uint8
	{
		Primitive,
		Enum,
		Struct,
		Array,
		Pointer,
		Reference,
		Container,
		Unknown
	};

	enum class EContainerType : uint8
	{
		Vector3,
		Vector4,
		Quat,
		Matrix3,
		Matrix4,
		String,
		Vector,
		Map,
		Set,
		Unknown
	};

	enum class EPrimitiveType : uint8
	{
		Bool,
		Int8, UInt8,
		Int16, UInt16,
		Int32, UInt32,
		Int64, UInt64,
		Float,
		Double,
		Unknown
	};

	enum class EFunctionType
	{
		Unknown,
		GetterFunc,
		SetterFunc
	};

	enum class Editability : uint8
	{
		Editable = 1 << 0,
		ReadOnly = 1 << 1
	};

	struct FieldInfo
	{
		std::string name{};												// 变量名
		std::string typeName{};											// 类型名(如果类型是别名则显示类型别名)
		std::string ownerClassName{};									// 所属类名
		std::string editableEnumStr;									// 是否可更改

		bool hasGetterMethod = false;
		std::string getterMethodName{};
		bool hasSetterMethod = false;
		std::string setterMethodName{};

		ETypeCategory category = ETypeCategory::Unknown;				// 变量分类
		EPrimitiveType primitiveType = EPrimitiveType::Unknown;			// 变量具体类型(仅当category==Primitive才有效)
		EContainerType containerType = EContainerType::Unknown;			// 容器类型(仅当category==Container才有效)

		const bool IsValid() const
		{
			return !name.empty() && !typeName.empty();
		}
	};

	struct ClassInfo
	{
		std::string className;
		std::string classPath;
		uint32 macroDefinedLine;
		std::vector<std::string> superClassName;
		std::vector<FieldInfo> fields;

		const bool IsValid() const
		{
			return !className.empty() && !classPath.empty();
		}
	};
}