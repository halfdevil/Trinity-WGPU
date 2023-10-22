cmake_minimum_required(VERSION 3.8)

project("Trinity-ImGui" CXX C)

file(GLOB_RECURSE COMMON_HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../Common/*.h")
file(GLOB_RECURSE COMMON_SOURCE_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../Common/*.c??")
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
file(GLOB_RECURSE SOURCE_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c??")

add_executable("Trinity-ImGui" ${COMMON_HEADER_FILES} ${COMMON_SOURCE_FILES} ${SOURCE_FILES} ${HEADER_FILES})

set_property(TARGET "Trinity-ImGui" PROPERTY CXX_STANDARD 20)
set_property(TARGET "Trinity-ImGui" PROPERTY CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIRS "Include" "../Common/Include")
set(COMPILE_DEFS "")
set(LINK_OPTIONS "")
set(LINK_LIBRARIES "Trinity-Framework")

if (CMAKE_SYSTEM_NAME MATCHES Emscripten)
	set(LINK_OPTIONS ${LINK_OPTIONS} "--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/Assets@/Assets")
endif()

target_include_directories("Trinity-ImGui" PRIVATE ${INCLUDE_DIRS})
target_compile_definitions("Trinity-ImGui" PRIVATE ${COMPILE_DEFS})
target_link_libraries("Trinity-ImGui" PRIVATE ${LINK_LIBRARIES} ${LINK_OPTIONS})

file(COPY "../../Assets/Framework" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/Assets")

if (CMAKE_SYSTEM_NAME MATCHES Emscripten)
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/AppConfig.json" "${CMAKE_CURRENT_SOURCE_DIR}/Assets" COPYONLY)
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/index.html" "${CMAKE_CURRENT_BINARY_DIR}/index.html" COPYONLY)
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/style.css" "${CMAKE_CURRENT_BINARY_DIR}/style.css" COPYONLY)
else()
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/Assets" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/AppConfig.json" ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endif()
