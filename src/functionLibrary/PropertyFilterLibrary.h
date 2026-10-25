#pragma once

#include "struct/ReflectStructInfo.h"

#include <clang/AST/DeclCXX.h>
#include <unordered_set>

namespace clang
{
	class SourceManager;
	class AnnotateAttr;
}

namespace suz
{
	class PropertyFilterLibrary
	{
	public:
		/* 遍历并构造符合的字段进入Map集合中 */
		static void ConstructClassInfo(
			clang::CXXRecordDecl* RecordDecl,
			const clang::SourceManager& SourceManager,
			const std::unordered_map<std::string, unsigned int>& MacroExpandLine,
			std::unordered_map<std::string, ClassInfo>& OutClassInfoMap
		);

	private:
		/* 递归提取字段 */
		static void ExtractFields(
			clang::CXXRecordDecl* RecordDecl,
			const clang::SourceManager& SourceManager,
			ClassInfo& OutClassInfo,
			std::unordered_set<const clang::CXXRecordDecl*>& VisitedClassSet,
			const std::string& ParentClassName = {}
		);

		static void ReadHasAnnotationField(clang::CXXRecordDecl* RecordDecl, ClassInfo& OutClassInfo, const std::string& ParentClassName);

		static void ReadAnnotationArgs(const clang::AnnotateAttr* AnnotateAttr, FieldInfo& OutFieldInfo);

		static void GainPropertyTypeEnum(const clang::QualType& QType, FieldInfo& OutFieldInfo);
	};
}