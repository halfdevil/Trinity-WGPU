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

if (CMAKE_SYSTEM_NAME MATCHES Emscripten)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fexperimental-library")
endif()

if (NOT CMAKE_SYSTEM_NAME MATCHES Emscripten)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

	add_subdirectory("ThirdParty/glfw")
	add_subdirectory("ThirdParty/dawnbuild")
	add_subdirectory("ThirdParty/optick")
	add_subdirectory("ThirdParty/CLI11")

	set_property(TARGET glfw PROPERTY FOLDER "ThirdParty")
	set_property(TARGET dawnbuild PROPERTY FOLDER "ThirdParty")
	set_property(TARGET OptickCore PROPERTY FOLDER "ThirdParty")

	include_directories("ThirdParty/glfw/include")
	include_directories("ThirdParty/dawnbuild/include")
	include_directories("ThirdParty/optick/src")
	include_directories("ThirdParty/CLI11/include") 
endif()

set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "" FORCE)
set(TINYGLTF_HEADER_ONLY ON CACHE BOOL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE BOOL "" FORCE)

set(MESHOPT_BUILD_DEMO OFF CACHE BOOL "")
set(MESHOPT_BUILD_TOOLS OFF CACHE BOOL "")
set(MESHOPT_BUILD_SHARED_LIBS OFF CACHE BOOL "")

add_subdirectory("ThirdParty/tinygltf")
add_subdirectory("ThirdParty/meshoptimizer")
add_subdirectory("ThirdParty/json")
add_subdirectory("ThirdParty/imguibuild")

set_property(TARGET nlohmann_json PROPERTY FOLDER "ThirdParty")
set_property(TARGET meshoptimizer PROPERTY FOLDER "ThirdParty")

include_directories("ThirdParty/stb")
include_directories("ThirdParty/glm")
include_directories("ThirdParty/tinygltf")
include_directories("ThirdParty/meshoptimizer/src")
include_directories("ThirdParty/json/include")
include_directories("ThirdParty/imgui")
include_directories("ThirdParty/IconFontCppHeaders")

# Include sub-projects.
add_subdirectory("Framework")
add_subdirectory("Samples")

if (NOT CMAKE_SYSTEM_NAME MATCHES Emscripten)
	add_subdirectory("Tools")
endif()
