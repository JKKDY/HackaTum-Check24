cmake_minimum_required(VERSION 3.10)

# Define the project name and version
project(HackaTum_Check24_Challenge VERSION 0.0.1)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release")

# Set optimization flags for release build
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3" CACHE STRING "Release build optimization level" FORCE)
endif()

# Enable AddressSanitizer for Debug builds
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

# Collect all cpp files
file(GLOB_RECURSE MY_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        )

# Create executable target
add_executable(HackCheck ${MY_SRC})

# Set C++ standard
target_compile_features(HackCheck
        PRIVATE
        cxx_std_17
        )

# Include source directory
target_include_directories(HackCheck
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

# Compiler options
target_compile_options(HackCheck
        PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:Intel>:-w3 -wd383,981,1418,1572,2259>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
        )

# Find and link Pistache library
find_package(PkgConfig REQUIRED)
pkg_check_modules(Pistache REQUIRED IMPORTED_TARGET libpistache)

target_link_libraries(HackCheck PkgConfig::Pistache)

# FetchContent for nlohmann/json
include(FetchContent)
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2  # You can specify a stable version here
)

FetchContent_MakeAvailable(nlohmann_json)

# Link nlohmann_json to the project
target_link_libraries(HackCheck PRIVATE nlohmann_json::nlohmann_json)