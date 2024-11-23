cmake_minimum_required(VERSION 3.10)

include(FetchContent)
# define project name, version
project(HackaTum_Check24_Challenge VERSION 0.0.1)


set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release")

# set optimization flags for release build
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3" CACHE STRING "Release build optimization level" FORCE)
endif ()

# Enable AdressSanitizer for Debug builds
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif ()

# collect all cpp files
file(GLOB_RECURSE MY_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        # header don't need to be included but this might be necessary for some IDEs
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# create make target
add_executable(HackCheck ${MY_SRC}
        src/database/DataBase.h
        src/io/Endpoints.h)

# set cxx standard.
target_compile_features(HackCheck
        PRIVATE
        cxx_std_20
)

target_include_directories(HackCheck
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(HackCheck
        PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:Intel>:-w3 -wd383,981,1418,1572,2259>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)


find_package(PkgConfig)
pkg_check_modules(Pistache REQUIRED IMPORTED_TARGET libpistache)

target_link_libraries(HackCheck PkgConfig::Pistache)


