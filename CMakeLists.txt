# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project("Trinity" CXX C)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY CXX_STANDARD 20)
set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if (NOT CMAKE_SYSTEM_NAME MATCHES Emscripten)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
	set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
	set(GLFW_INSTALL OFF CACHE BOOL "")

	add_subdirectory("ThirdParty/glfw")
	add_subdirectory("ThirdParty/dawnbuild")

	set_property(TARGET glfw PROPERTY FOLDER "ThirdParty")
	set_property(TARGET dawnbuild PROPERTY FOLDER "ThirdParty")

	include_directories("ThirdParty/glfw/include")
	include_directories("ThirdParty/dawnbuild/include")
endif()

set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "")
set(TINYGLTF_HEADER_ONLY ON CACHE BOOL "")
set(TINYGLTF_INSTALL OFF CACHE BOOL "")

add_subdirectory("ThirdParty/tinygltf")

include_directories("ThirdParty/stb")
include_directories("ThirdParty/glm")
include_directories("ThirdParty/tinygltf")

# Include sub-projects.
add_subdirectory("Framework")
add_subdirectory("Samples")

