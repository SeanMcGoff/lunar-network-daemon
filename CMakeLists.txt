# CMakeLists.txt

cmake_minimum_required(VERSION 3.12)

project(
  lunar-network-daemon
  VERSION 0.1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add some flags to ensure a better quality of code.
# "-Wall" cehcks for many errors
set(CMAKE_CXX_FLAGS "-Wall")
# IN debug mode compile in debug and add sanitisers to check for ememory and undefined behaviour issues
# Slows program significantly (~2x slowdown) but fine for debug build
set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address,undefined")

# If no build type specified, build in debug.
set(default_build_type "Debug")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(
    STATUS
      "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE
      "${default_build_type}"
      CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "MinSizeRel" "RelWithDebInfo")
endif()
message(STATUS "Building with type '${CMAKE_BUILD_TYPE}'.")

# Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Allows CTest to find tests in test/ subdir
enable_testing()

add_subdirectory(src)
add_subdirectory(test)
