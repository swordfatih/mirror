cmake_minimum_required(VERSION 4.3.2)

set(CMAKE_CXX_MODULE_STD OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(mirror LANGUAGES CXX)

option(MIRROR_BUILD_DEMO "Build the mirror demo executable" OFF)

include(FetchContent)
include(CTest)
enable_testing()

# ------------------------
# Dependencies
# ------------------------
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build dependencies statically" FORCE)

set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(nlohmann-json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.12.0 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(nlohmann-json)

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(yaml-cpp GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git GIT_TAG yaml-cpp-0.9.0 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(yaml-cpp)

FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.14.0 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(Catch2)

if(MIRROR_BUILD_DEMO)
    set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.x GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(spdlog)
endif()

# ------------------------
# Configuration function
# ------------------------
function(configure_mirror_library target)
    set_target_properties(${target} PROPERTIES CXX_SCAN_FOR_MODULES OFF)
    target_include_directories(${target} PUBLIC ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${target} PRIVATE nlohmann_json::nlohmann_json yaml-cpp::yaml-cpp)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-but-set-variable -Wno-unused-parameter -freflection)
    if(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE -O1)
    endif()
endfunction()

function(configure_mirror_binary target)
    set_target_properties(${target} PROPERTIES CXX_SCAN_FOR_MODULES OFF)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-but-set-variable -Wno-unused-parameter -freflection)
endfunction()

# ------------------------
# Library
# ------------------------
file(GLOB_RECURSE cpp_sources CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

if(cpp_sources)
    add_library(${PROJECT_NAME}_lib STATIC)
    target_sources(${PROJECT_NAME}_lib PRIVATE ${cpp_sources})
    configure_mirror_library(${PROJECT_NAME}_lib)
endif()

# ------------------------
# Executable
# ------------------------
if(MIRROR_BUILD_DEMO)
    add_executable(${PROJECT_NAME}_demo ${CMAKE_SOURCE_DIR}/examples/demo.cpp)
    set_target_properties(${PROJECT_NAME}_demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/runtime)
    target_link_libraries(${PROJECT_NAME}_demo PRIVATE ${PROJECT_NAME}_lib spdlog::spdlog)
    target_link_options(${PROJECT_NAME}_demo PRIVATE -static -static-libgcc -static-libstdc++)
    configure_mirror_binary(${PROJECT_NAME}_demo)
    if(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${PROJECT_NAME}_demo PRIVATE -O1)
    endif()
endif()

# ------------------------
# Tests
# ------------------------
file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/*.cpp")

if(test_sources AND TARGET ${PROJECT_NAME}_lib)
    add_executable(${PROJECT_NAME}_tests ${test_sources})
    set_target_properties(${PROJECT_NAME}_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/runtime)
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${PROJECT_NAME}_lib Catch2::Catch2WithMain)
    configure_mirror_binary(${PROJECT_NAME}_tests)
    include(${catch2_SOURCE_DIR}/extras/Catch.cmake)
    catch_discover_tests(${PROJECT_NAME}_tests)
endif()
