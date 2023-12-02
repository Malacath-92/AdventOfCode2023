cmake_minimum_required(VERSION 3.22.3)
project(AdventOfCode2023 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
enable_testing()

# --------------------------------------------------
# Make sure we know which compiler we're using
string(COMPARE EQUAL "Clang" "${CMAKE_CXX_COMPILER_ID}" CLANG)
string(COMPARE EQUAL "Apple Clang" "${CMAKE_CXX_COMPILER_ID}" APPLE_CLANG)
string(COMPARE EQUAL "GNU" "${CMAKE_CXX_COMPILER_ID}" GNU)
if (MSVC)
	message("Detected MSVC...")
elseif (APPLE_CLANG)
	message("Detected Apple Clang...")
elseif (CLANG)
	message("Detected Clang...")
elseif(GNU)
	message("Detected GCC...")
else()
	message("Not sure what compiler you are running...")
endif()

# --------------------------------------------------
# Add formatting target
include(cmake/clang-format.cmake)
setup_format_target(format_aoc)

# --------------------------------------------------
# Run conan
include(cmake/conan.cmake)
run_conan()

# --------------------------------------------------
# Find packages
find_package(fmt QUIET REQUIRED)

# --------------------------------------------------
# Create interface libs
add_library(aoc_haxx INTERFACE)
if(MSVC)
	target_compile_options(aoc_haxx INTERFACE /constexpr:steps16777216 /constexpr:depth16777216)
elseif(CLANG)
	target_compile_options(aoc_haxx INTERFACE -fconstexpr-steps=16777216 -fbracket-depth=4096)
elseif(APPLE_CLANG)
	target_compile_options(aoc_haxx INTERFACE -fconstexpr-steps=16777216 -fbracket-depth=4096)
else()
	target_compile_options(aoc_haxx INTERFACE -fconstexpr-depth=16777216)
endif()

add_library(aoc_warnings INTERFACE)
if(MSVC)
	target_compile_options(aoc_warnings INTERFACE /W4 /WX /permissive-)
else()
	target_compile_options(aoc_warnings INTERFACE -Wall -Wextra -pedantic -Werror)
endif()

add_library(aoc_definitions INTERFACE)
target_compile_definitions(aoc_definitions INTERFACE
	_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
	WIN32_LEAN_AND_MEAN
	NOMINMAX)
set_property(
    TARGET aoc_definitions
    PROPERTY UNITY_BUILD ON)

add_library(aoc_dependencies INTERFACE)
target_link_libraries(aoc_dependencies INTERFACE
    fmt::fmt)

add_library(aoc_precompiled_headers INTERFACE)
target_precompile_headers(aoc_precompiled_headers INTERFACE
	<any>
	<string>
	<string_view>
	<array>
	<vector>
	<deque>
	<map>
	<unordered_map>
	<set>
	<unordered_set>
	<fstream>
	<filesystem>
	<variant>
	<functional>
	<algorithm>
	<iostream>
	<iomanip>
	<locale>
	<mutex>)

file(GLOB_RECURSE aoc_util_sources CONFIGURE_DEPENDS "util/*.cpp")
file(GLOB_RECURSE aoc_util_headers CONFIGURE_DEPENDS "util/*.h" "util/*.inl")
add_library(aoc_util STATIC
    ${aoc_util_sources} 
    ${aoc_util_headers})
target_include_directories(aoc_util PUBLIC
    "util")

# Create a target for each source file
file(GLOB aoc_main_files CONFIGURE_DEPENDS "*.cpp")
foreach(main_file ${aoc_main_files})
    get_filename_component(main_file_name ${main_file} NAME_WE)
    add_executable(${main_file_name} ${main_file})
    target_link_libraries(${main_file_name} PRIVATE
        aoc_haxx
        aoc_warnings
        aoc_definitions
        aoc_dependencies
        aoc_precompiled_headers
        aoc_util)
	set_target_properties(${main_file_name} PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		VS_DEBUGGER_COMMAND_ARGUMENTS "inputs/${main_file_name}.txt")
    add_test(
		NAME ${main_file_name}
		COMMAND ${main_file_name} "inputs/${main_file_name}.txt"
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endforeach()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT day1_1)
