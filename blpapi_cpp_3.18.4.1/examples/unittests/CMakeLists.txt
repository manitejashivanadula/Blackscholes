cmake_minimum_required(VERSION 3.15.2)
project(unittests LANGUAGES C CXX)

if (${CMAKE_SYSTEM_NAME} STREQUAL "SunOS")
    message(FATAL_ERROR "Mock examples cannot be built in SunOS")
endif()

# If no CMAKE_PREFIX_PATH was provided, add the expected location
# relative to the examples
if (NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")
endif()

message(STATUS "CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(_GTEST_DOWNLOAD googletest-download)
set(_GTEST_SRC googletest-src)
set(_GTEST_BUILD googletest-build)

include(CTest)
include(GoogleTest)

# By default build with Release configuration.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Find Blpapi
find_package(blpapi REQUIRED CONFIG)

# If GTEST_SRC_DIR is not provided download and unpack googletest at
# configure time.
if(NOT GTEST_SRC_DIR)
  configure_file(GTEST_CMakeLists.txt.in "${_GTEST_DOWNLOAD}/CMakeLists.txt")
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_GTEST_DOWNLOAD}")

 if(result)
   message(FATAL_ERROR "CMake step for googletest failed: ${result}")
 endif()

 execute_process(COMMAND ${CMAKE_COMMAND} --build .
   RESULT_VARIABLE result
   WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_GTEST_DOWNLOAD}")

 if(result)
   message(FATAL_ERROR "Build step for googletest failed: ${result}")
 endif()

  set(GTEST_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/${_GTEST_SRC})
  set(GTEST_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/${_GTEST_BUILD})
else()
  set(GTEST_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/${_GTEST_BUILD})
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory("${GTEST_SRC_DIR}" "${GTEST_BUILD_DIR}" EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

add_subdirectory(mktnotifier)
add_subdirectory(snippets)
