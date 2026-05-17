cmake_minimum_required(VERSION 4.3.2)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
set(CMAKE_CXX_MODULE_STD ON)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(mirror LANGUAGES CXX)

include(FetchContent)
include(CTest)
enable_testing()

# ------------------------
# Dependencies
# ------------------------
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build dependencies statically" FORCE)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)

FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.x GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(spdlog)

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

if(TARGET Catch2)
    target_compile_options(Catch2 PRIVATE -O1)
endif()

if(TARGET Catch2WithMain)
    target_compile_options(Catch2WithMain PRIVATE -O1)
endif()

# ------------------------
# Configuration function
# ------------------------
function(configure_target target)
    target_include_directories(${target} PUBLIC ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${target} PUBLIC spdlog::spdlog)
    target_link_libraries(${target} PUBLIC nlohmann_json::nlohmann_json)
    if(TARGET yaml-cpp::yaml-cpp)
        target_link_libraries(${target} PUBLIC yaml-cpp::yaml-cpp)
    else()
        target_link_libraries(${target} PUBLIC yaml-cpp)
    endif()
    target_compile_options(${target} PUBLIC -Wall -Wextra -Wpedantic -Wno-unused-but-set-variable -Wno-unused-parameter -fmodules -freflection)
    target_link_options(${target} PUBLIC -static -static-libgcc -static-libstdc++)
    target_compile_options(${target} PUBLIC -O1)
endfunction()

# ------------------------
# Library
# ------------------------
file(GLOB_RECURSE cpp_sources CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE module_sources CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.mpp")
list(FILTER cpp_sources EXCLUDE REGEX ".*/main\\.cpp$")

if(cpp_sources OR module_sources)
    add_library(${PROJECT_NAME}_lib STATIC)

    if(cpp_sources)
        target_sources(${PROJECT_NAME}_lib PRIVATE ${cpp_sources})
    endif()

    if(module_sources)
        target_sources(${PROJECT_NAME}_lib
            PUBLIC
                FILE_SET cxx_modules
                TYPE CXX_MODULES
                BASE_DIRS "${CMAKE_SOURCE_DIR}/src"
                FILES ${module_sources}
        )
    endif()

    configure_target(${PROJECT_NAME}_lib)
endif()

# ------------------------
# Executable
# ------------------------
add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/runtime)

if(TARGET ${PROJECT_NAME}_lib)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_lib)
else()
    configure_target(${PROJECT_NAME})
endif()

# ------------------------
# Tests
# ------------------------
file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/*.cpp")

if(test_sources AND TARGET ${PROJECT_NAME}_lib)
    add_executable(${PROJECT_NAME}_tests ${test_sources})
    set_target_properties(${PROJECT_NAME}_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/runtime)
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${PROJECT_NAME}_lib Catch2::Catch2WithMain)
    configure_target(${PROJECT_NAME}_tests)
    include(${catch2_SOURCE_DIR}/extras/Catch.cmake)
    catch_discover_tests(${PROJECT_NAME}_tests)
endif()
