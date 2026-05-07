cmake_minimum_required(VERSION 4.3.2)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
set(CMAKE_CXX_MODULE_STD ON)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(reflection LANGUAGES CXX)

include(FetchContent)

# ------------------------
# Dependencies
# ------------------------
FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.x GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(spdlog)

# ------------------------
# Configuration function
# ------------------------
function(configure_target target)
    target_include_directories(${target} PUBLIC ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${target} PUBLIC spdlog::spdlog)
    target_compile_options(${target} PUBLIC -Wall -Wextra -Wpedantic -Wno-unused-but-set-variable -Wno-unused-parameter -fmodules -freflection)
    target_link_options(${target} PUBLIC -static -static-libgcc -static-libstdc++)
endfunction()

# ------------------------
# Library
# ------------------------
file(GLOB_RECURSE library_sources "${CMAKE_SOURCE_DIR}/src/**.cpp")
list(FILTER library_sources EXCLUDE REGEX ".*/main\\.cpp$")

if(library_sources)
    add_library(${PROJECT_NAME}_lib STATIC ${library_sources})
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
