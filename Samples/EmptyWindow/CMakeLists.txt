cmake_minimum_required(VERSION 3.12)

project("Trinity-EmptyWindow" CXX C)

file(GLOB_RECURSE COMMON_HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../Common/*.h")
file(GLOB_RECURSE COMMON_SOURCE_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../Common/*.c??")
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
file(GLOB_RECURSE SOURCE_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c??")

add_executable("Trinity-EmptyWindow" ${COMMON_HEADER_FILES} ${COMMON_SOURCE_FILES} ${SOURCE_FILES} ${HEADER_FILES})

set_property(TARGET "Trinity-EmptyWindow" PROPERTY CXX_STANDARD 20)
set_property(TARGET "Trinity-EmptyWindow" PROPERTY CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIRS "Include" "../Common/Include")
set(COMPILE_DEFS "")
set(LINK_OPTIONS "")
set(LINK_LIBRARIES "Trinity-Framework")

target_include_directories("Trinity-EmptyWindow" PRIVATE ${INCLUDE_DIRS})
target_compile_definitions("Trinity-EmptyWindow" PRIVATE ${COMPILE_DEFS})
target_link_libraries("Trinity-EmptyWindow" PRIVATE ${LINK_LIBRARIES} ${LINK_OPTIONS})

if (CMAKE_SYSTEM_NAME MATCHES Emscripten)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/index.html ${CMAKE_CURRENT_BINARY_DIR}/index.html COPYONLY)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/style.css ${CMAKE_CURRENT_BINARY_DIR}/style.css COPYONLY)
endif()
