
cmake_minimum_required (VERSION 3.8)

set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 20)

project (GENERATED_REFLECT_CODE_PROGRAM)

add_subdirectory(shared)

# 在正式发布时将此设置为ON而不是OFF
# 设置是否为生产构建
# Make Sure PRODUCTION_BUILD WHEN SEND TO INTERNET
set(PRODUCTION_BUILD OFF CACHE BOOL "Make this a production build")

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

# 关闭LLVM警告
set(LLVM_ENABLE_WARNINGS OFF CACHE BOOL "Disable all llvm warnings")
set(LLVM_ENABLE_WERROR OFF CACHE BOOL "Warnings are not errors")

file(GLOB_RECURSE GENERATED_REFLECT_CODE CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# --------------------------- 生成配置文件 ---------------------------
set(LLVM_INCLUDE_DIR "${LLVM_INCLUDE_DIRS}")
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/shared/SuzzaGenerateconfig.h.in
	${CMAKE_CURRENT_BINARY_DIR}/shared/SuzzaGenerateconfig.h
)
# -------------------------------------------------------------------


# 加入LLVM头文件搜索路径
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

add_definitions(${LLVM_DEFINITIONS})
add_definitions(${CLANG_DEFINITIONS})

add_executable("${CMAKE_PROJECT_NAME}")
# 设置头文件目录和源码目录
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE "${GENERATED_REFLECT_CODE}")
# Include Binary Files
target_include_directories("${CMAKE_PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
)


# ---------------------- 生产环境变量配置 -----------------------
if(PRODUCTION_BUILD)

	# 设置生产环境变量, 处于生产环境时为1
	# PRODUCTION_BUILD value is 1 WHEN IN PRODUCTION_BUILD
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1)

else()
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0)

endif()
# -------------------------------------------------------------


# LLVM组件
llvm_map_components_to_libnames(LLVM_LIBS
	core
	support
)
# 库搜索路径
target_link_directories("${CMAKE_PROJECT_NAME}" PRIVATE 
	${LLVM_LIBRARY_DIRS}
    ${CLANG_LIBRARY_DIRS}
)
# 链接外部库
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE
	Suzza_Shared_Moudle
	${LLVM_LIBS}
	clangTooling
)
# 强制使用Release的CRT, 即使在Debug环境下
target_compile_options("${CMAKE_PROJECT_NAME}" PRIVATE /Zi)
target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE /DEBUG)