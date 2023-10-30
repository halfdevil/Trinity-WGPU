cmake_minimum_required(VERSION 3.8)

project("Trinity-SkyboxTool" CXX C)

file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
file(GLOB_RECURSE SOURCE_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c??")

add_executable("Trinity-SkyboxTool" ${SOURCE_FILES} ${HEADER_FILES})

set_property(TARGET "Trinity-SkyboxTool" PROPERTY CXX_STANDARD 20)
set_property(TARGET "Trinity-SkyboxTool" PROPERTY CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIRS "Include")
set(COMPILE_DEFS "")
set(LINK_OPTIONS "")
set(LINK_LIBRARIES "Trinity-Framework")

target_include_directories("Trinity-SkyboxTool" PRIVATE ${INCLUDE_DIRS})
target_compile_definitions("Trinity-SkyboxTool" PRIVATE ${COMPILE_DEFS})
target_link_libraries("Trinity-SkyboxTool" PRIVATE ${LINK_LIBRARIES} ${LINK_OPTIONS})
