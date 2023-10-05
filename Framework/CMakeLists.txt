cmake_minimum_required (VERSION 3.8)

project("Trinity-Framework" CXX C)

file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
file(GLOB_RECURSE SOURCE_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c??)

add_library("Trinity-Framework" ${SOURCE_FILES} ${HEADER_FILES}
	${GENERATED_HEADER_FILES} ${GENERATED_SOURCE_FILES})

set_property(TARGET "Trinity-Framework" PROPERTY CXX_STANDARD 20)
set_property(TARGET "Trinity-Framework" PROPERTY CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIRS "Include")
set(COMPILE_DEFS "-DGLM_ENABLE_EXPERIMENTAL")
set(LINK_OPTIONS "")
set(LINK_LIBRARIES "glfw" "tinygltf")

if (CMAKE_BUILD_TYPE MATCHES Debug)
	set(COMPILE_DEFS ${COMPILE_DEFS} DEBUG_BUILD=1)  
endif()

if (MSVC)
	set(COMPILE_DEFS ${COMPILE_DEFS} -D_CONSOLE)
endif()

if (CMAKE_SYSTEM_NAME MATCHES Emscripten)
	set(LINK_OPTIONS ${LINK_OPTIONS} "-sALLOW_MEMORY_GROWTH" "-sUSE_GLFW=3" "-sUSE_WEBGPU" "--bind")
else()
	set(LINK_LIBRARIES ${LINK_LIBRARIES} "dawnbuild")
endif()

target_include_directories("Trinity-Framework" PUBLIC ${INCLUDE_DIRS})
target_compile_definitions("Trinity-Framework" PUBLIC ${COMPILE_DEFS})
target_link_libraries("Trinity-Framework" PUBLIC ${LINK_LIBRARIES} ${LINK_OPTIONS})