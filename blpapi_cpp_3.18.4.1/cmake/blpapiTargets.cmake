if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.5)
  message(FATAL_ERROR "CMake >= 2.6 required")
endif()

cmake_policy(PUSH)
cmake_policy(VERSION 2.6)

# Protect against multiple inclusion, which would fail when already
# imported targets are added once more.
set(_targetsDefined)
set(_targetsNotDefined)
set(_expectedTargets)

foreach (_expectedTarget ${_TARGETS})
  list(APPEND _expectedTargets ${_expectedTarget})
  if (NOT TARGET ${_expectedTarget})
    list(APPEND _targetsNotDefined ${_expectedTarget})
  endif()

  if (TARGET ${_expectedTarget})
    list(APPEND _targetsDefined ${_expectedTarget})
  endif()
endforeach()

# Requested targets are already defined, return.
if ("${_targetsDefined}" STREQUAL ${_expectedTargets})
  unset(_targetsDefined)
  unset(_targetsNotDefined)
  unset(_expectedTargets)
  cmake_policy(POP)
  return()
endif()

# If some of the targets are defined, something went wrong.
# Inform user and fail.
if (NOT "${_targetsDefined}" STREQUAL "")
  message(FATAL_ERROR
    "Some (but not all) targets in this export set were already defined.\n\
     Targets Defined: ${_targetsDefined}\
     Targets not yet defined: ${_targetsNotDefined}\n")
endif()

unset(_targetsDefined)
unset(_targetsNotDefined)
unset(_expectedTargets)

# Create imported shared object target blpapi.
add_library(blpapi SHARED IMPORTED)

# Include headers can be found at ../include/ from location
# of blpapiConfig.cmake file.
set(_BLPAPI_LIB_INCLUDE_DIR "${_BLPAPI_CONFIG_CMAKE_DIR}/../include")

# Add path to the include directories.
set_property(TARGET blpapi APPEND PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES "${_BLPAPI_LIB_INCLUDE_DIR}")

list(APPEND _IMPORT_CHECK_FILES_FOR_blpapi "${_BLPAPI_LIB_INCLUDE_DIR}")

# Compute the architecture before loading configuration information.
# It is necessary because the architecture is needed for blpapi shared
# object name and its location.
math(EXPR _ARCH "8 * ${CMAKE_SIZEOF_VOID_P}")

# Set library name prefix
set(_BLPAPI_PREFIX "blpapi3")

# Load information for each installed configuration.
file(GLOB CONFIG_FILES "${_BLPAPI_CONFIG_CMAKE_DIR}/blpapiTargets-*.cmake")
foreach(f ${CONFIG_FILES})
  include(${f})
endforeach()

# Loop over all imported files and verify that they actually exist
foreach(target ${_TARGETS})
  foreach(file ${_IMPORT_CHECK_FILES_FOR_${target}} )
    if(NOT EXISTS "${file}" )
      message(FATAL_ERROR "The imported target \"${target}\" references the file
              \"${file}\"
but this file does not exist.  Possible reasons include:
* The file was deleted, renamed, or moved to another location.
* An install or uninstall procedure did not complete successfully.
* The installation package was faulty and contained
\"${CMAKE_CURRENT_LIST_FILE}\"
not all the files it references.")
    endif()
  endforeach()
  unset(_IMPORT_CHECK_FILES_FOR_${target})
endforeach()

# Clean up internal variables
unset(_ARCH)
unset(_BLPAPI_PREFIX)
unset(_BLPAPI_LIB_INCLUDE_DIR)
cmake_policy(POP)
