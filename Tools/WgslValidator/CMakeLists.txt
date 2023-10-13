cmake_minimum_required(VERSION 3.8)

project("Trinity-WgslValidator" CXX C)

file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
file(GLOB_RECURSE SOURCE_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c??")

add_executable("Trinity-WgslValidator" ${SOURCE_FILES} ${HEADER_FILES})

set_property(TARGET "Trinity-WgslValidator" PROPERTY CXX_STANDARD 20)
set_property(TARGET "Trinity-WgslValidator" PROPERTY CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIRS "Include")
set(COMPILE_DEFS "")
set(LINK_OPTIONS "")
set(LINK_LIBRARIES "Trinity-Framework")

target_include_directories("Trinity-WgslValidator" PRIVATE ${INCLUDE_DIRS})
target_compile_definitions("Trinity-WgslValidator" PRIVATE ${COMPILE_DEFS})
target_link_libraries("Trinity-WgslValidator" PRIVATE ${LINK_LIBRARIES} ${LINK_OPTIONS})
