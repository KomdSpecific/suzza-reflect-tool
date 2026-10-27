#include "GenerateReflectString.h"

#include <sstream>

namespace suz
{
	static std::string GenerateFieldCode(const FieldInfo& InFieldInfo, const std::string& ClassName, bool IsLastField);
	static std::string GenerateCategoryEnumStr(const FieldInfo& InFIeldInfo);
	static std::string GenerateContainerEnumStr(EContainerType ContainerType);
	static std::string GeneratePrimitiveEnumStr(EPrimitiveType PrimitiveType);

	std::string GenerateReflectString::GenerateClassHeaderString(
		const std::pair<std::string, ClassInfo>& ClassData,
		const std::string& ClassName
	)
	{
		std::stringstream stream;

		stream << "#pragma once" << "\n" << std::endl;
		stream << R"(#include "macros/ReflectMacros.h")" << std::endl;
		stream << R"(#include "ReflectionInfo.h")" << "\n" << std::endl;
		stream << "#undef " << "SUZZA_GENERATED_NAME" << std::endl;
		stream << "#define " << "SUZZA_GENERATED_NAME " << ClassName + "_REFLECT_CLASS" << "\n";
		stream << "#define " << ClassName + "_REFLECT_CLASS_" << ClassData.second.macroDefinedLine << "_REFLECT_BODY \\" << std::endl;
		stream << "	using ThisClass = " << ClassName << "; \\" << std::endl;

		/* Super只能设置一个父类, 统一规定主父类Super是public之后的第一个类 */
		if (ClassData.second.superClassName.size() > 0)
		{
			stream << "	using Super = " << ClassData.second.superClassName[0] << "; \\" << std::endl;
		}

		stream << "	static const ClassInfo m_" << ClassName << "ClassInfo; \\" << std::endl;
		stream << "	friend struct FieldInfo; \\" << std::endl;
		stream << "public: \\" << std::endl;
		stream << "	static const ClassInfo& StaticClass(); \\" << std::endl;

		if (ClassData.second.superClassName.size() > 0)
		{
			stream << "	virtual const ClassInfo& GetClassInfo() const override;";
		}
		else
		{
			stream << "	virtual const ClassInfo& GetClassInfo() const;";
		}

		return stream.str();
	}

	std::string GenerateReflectString::GenerateClassSourceString(
		const std::pair<std::string, ClassInfo>& ClassData,
		const std::string& IncludeClassPath,
		const std::string& ClassName,
		const std::string& ClassPath
	)
	{
		std::stringstream stream;

		stream << "#include " << "\"" << IncludeClassPath << "\"" << "\n" << std::endl;

		stream << "namespace suz" << "\n" << "{" << std::endl;
		stream << "	const ClassInfo " << ClassName << "::m_" << ClassName << "ClassInfo = {" << std::endl;
		stream << "		\"" << ClassName << "\"," << std::endl;
		stream << "		\"" << ClassPath << "\"," << std::endl;

		if (ClassData.second.superClassName.size() > 0)
		{
			stream << "		{\"";
			for (usize i = 0; i < ClassData.second.superClassName.size(); i++)
			{
				if (i == ClassData.second.superClassName.size() - 1)
				{
					stream << ClassData.second.superClassName[i] << "\"}," << std::endl;
					continue;
				}

				stream << ClassData.second.superClassName[i] << "\", \"";
			}
		}
		else
		{
			stream << "		{}," << std::endl;
		}

		if (ClassData.second.fields.size() != 0)
		{
			stream << "		{\n			";
			for (usize i = 0; i < ClassData.second.fields.size(); i++)
			{
				FieldInfo currentField = ClassData.second.fields[i];

				if (i == ClassData.second.fields.size() - 1)
				{
					/* 当前为最后一个元素 */
					stream << GenerateFieldCode(currentField, ClassName, true);
					continue;
				}

				stream << GenerateFieldCode(currentField, ClassName, false);
			}
		}
		else
		{
			stream << "		{}\n	};\n\n";
		}

		stream << "	const ClassInfo& " << ClassName << "::StaticClass()\n	{\n		return m_" << ClassName << "ClassInfo;" << std::endl;
		stream << "	}\n" << std::endl;

		stream << "	const ClassInfo& " << ClassName << "::GetClassInfo() const\n	{\n		return m_" << ClassName << "ClassInfo;" << std::endl;
		stream << "	}\n}" << std::endl;

		return stream.str();
	}

	std::string GenerateReflectString::GenerateContainerCode(const FieldInfo& InFieldInfo)
	{
		std::stringstream stream;

		switch (InFieldInfo.containerType)
		{
		case EContainerType::Vector3:

		default:
			break;
		}

		return std::string();
	}

	std::string GenerateFieldCode(const FieldInfo& InFieldInfo, const std::string& ClassName, bool IsLastField)
	{
		std::stringstream stream;

		std::string fieldEnumStr = GenerateCategoryEnumStr(InFieldInfo);

		/* 字段为本类的字段 */
		if (InFieldInfo.parentClassName.empty())
		{
			stream << "FieldInfo::CreateFieldInfo(";
			stream << "\"" << InFieldInfo.name << "\", ";
			stream << "\"" << InFieldInfo.typeName << "\", ";
			stream << "SUZZA_OFFSET_OF(" << ClassName + "::" + InFieldInfo.name << ")" << ", ";
			stream << "sizeof(" << InFieldInfo.typeName << "), ";
			stream << fieldEnumStr;
			if (IsLastField)
			{
				stream << InFieldInfo.editableEnumStr << ")\n		}\n	};\n\n";
			}
			else
			{
				stream << InFieldInfo.editableEnumStr << "),\n			";
			}
		}
		else
		{
			if (!InFieldInfo.hasGetterMethod) return {};

			stream << "FieldInfo::CreateFieldInfo(";
			stream << "\"" << InFieldInfo.name << "\", ";
			stream << "\"" << InFieldInfo.typeName << "\", ";
			stream << "\"" << InFieldInfo.parentClassName << "\", ";

			stream << "&" << InFieldInfo.parentClassName << "::" << InFieldInfo.getterMethodName << ", ";
			if (InFieldInfo.hasSetterMethod)
			{
				stream << "&" << InFieldInfo.parentClassName << "::" << InFieldInfo.setterMethodName << ", ";
			}
			else
			{
				stream << "nullptr";
			}

			stream << fieldEnumStr;
			if (IsLastField)
			{
				stream << InFieldInfo.editableEnumStr << ")\n		}\n	};\n\n";
			}
			else
			{
				stream << InFieldInfo.editableEnumStr << "),\n			";
			}
		}

		return stream.str();
	}

	std::string GenerateCategoryEnumStr(const FieldInfo& InFIeldInfo)
	{
		switch (InFIeldInfo.category)
		{
		case ETypeCategory::Primitive:
			return "ETypeCategory::Primitive, " + GeneratePrimitiveEnumStr(InFIeldInfo.primitiveType);

		case ETypeCategory::Enum:
			return "ETypeCategory::Enum, ";

		case ETypeCategory::Struct:
			return "ETypeCategory::Struct, ";

		case ETypeCategory::Array:
			return "ETypeCategory::Array, ";

		case ETypeCategory::Pointer:
			return "ETypeCategory::Pointer, ";

		case ETypeCategory::Reference:
			return "ETypeCategory::Reference, ";

		case ETypeCategory::Container:
			return "ETypeCategory::Container, " + GenerateContainerEnumStr(InFIeldInfo.containerType);

		case ETypeCategory::Unknown:
			return "ETypeCategory::Unknown, ";

		default:
			return "ETypeCategory::Unknown, ";
		}
	}

	std::string GenerateContainerEnumStr(EContainerType ContainerType)
	{
		switch (ContainerType)
		{
		case EContainerType::Vector3:
			return "EContainerType::Vector3, ";

		case EContainerType::Vector4:
			return "EContainerType::Vector4, ";

		case EContainerType::Quat:
			return "EContainerType::Quat, ";

		case EContainerType::Matrix3:
			return "EContainerType::Matrix3, ";

		case EContainerType::Matrix4:
			return "EContainerType::Matrix4, ";

		case EContainerType::String:
			return "EContainerType::String, ";

		case EContainerType::Vector:
			return "EContainerType::Vector, ";

		case EContainerType::Map:
			return "EContainerType::Map, ";

		case EContainerType::Set:
			return "EContainerType::Set, ";

		case EContainerType::Unknown:
			return "EContainerType::Unknown, ";

		default:
			return "EContainerType::Unknown, ";
		}
	}

	std::string GeneratePrimitiveEnumStr(EPrimitiveType PrimitiveType)
	{
		switch (PrimitiveType)
		{
		case EPrimitiveType::Bool:
			return "EPrimitiveType::Bool, ";

		case EPrimitiveType::Int8:
			return "EPrimitiveType::Int8, ";

		case EPrimitiveType::UInt8:
			return "EPrimitiveType::UInt8, ";

		case EPrimitiveType::Int16:
			return "EPrimitiveType::Int16, ";

		case EPrimitiveType::UInt16:
			return "EPrimitiveType::UInt16, ";

		case EPrimitiveType::Int32:
			return "EPrimitiveType::UInt32, ";

		case EPrimitiveType::UInt32:
			return "EPrimitiveType::UInt32, ";

		case EPrimitiveType::Int64:
			return "EPrimitiveType::UInt64, ";

		case EPrimitiveType::UInt64:
			return "EPrimitiveType::UInt64, ";

		case EPrimitiveType::Float:
			return "EPrimitiveType::Float, ";

		case EPrimitiveType::Unknown:
			return "EPrimitiveType::Unknown, ";

		default:
			return "EPrimitiveType::Unknown, ";
		}
	}
}