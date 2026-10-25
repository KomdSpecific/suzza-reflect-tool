cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project (Suzza_Shared_Moudle)

file(GLOB_RECURSE SHARED_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/file/*.cpp")

add_library("${PROJECT_NAME}")
target_sources("${PROJECT_NAME}" PRIVATE ${SHARED_SOURCES})

# Include Binary Files
target_include_directories("${PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

target_include_directories("${PROJECT_NAME}" PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
)