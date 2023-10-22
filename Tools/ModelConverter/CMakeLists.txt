cmake_minimum_required(VERSION 3.8)

project("Trinity-ModelConverter" CXX C)

file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
file(GLOB_RECURSE SOURCE_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c??")

add_executable("Trinity-ModelConverter" ${SOURCE_FILES} ${HEADER_FILES})

set_property(TARGET "Trinity-ModelConverter" PROPERTY CXX_STANDARD 20)
set_property(TARGET "Trinity-ModelConverter" PROPERTY CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIRS "Include")
set(COMPILE_DEFS "")
set(LINK_OPTIONS "")
set(LINK_LIBRARIES "Trinity-Framework")

target_include_directories("Trinity-ModelConverter" PRIVATE ${INCLUDE_DIRS})
target_compile_definitions("Trinity-ModelConverter" PRIVATE ${COMPILE_DEFS})
target_link_libraries("Trinity-ModelConverter" PRIVATE ${LINK_LIBRARIES} ${LINK_OPTIONS})
