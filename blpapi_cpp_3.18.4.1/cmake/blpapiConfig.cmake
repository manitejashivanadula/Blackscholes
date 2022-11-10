# blpapiConfig.cmake
# ------------------
#
# blpapi cmake module.
# This module sets the following variables in your project:
#
# ::
#
#    blpapi_FOUND - true if blpapi found on the system
#
#
# Exported target:
#
# ::
#
# If blpapi is found, this module defines the following :prop_tgt:`IMPORTED`
# target. ::
#    blpapi - the main blpapi shared library with headers and defs attached.
#
# Suggested usage:
#
# ::
#    find_package(blpapi REQUIRED CONFIG)
#
#
# The following variables can be set to guide the search for this package:
#
# ::
#    blpapi_DIR - CMake variable, set to directory containing this Config file.
#    CMAKE_PREFIX_PATH - CMake variable, set to root directory (cmake) of this
#                        package.

# Check if the required targets are successfully exported and set up
# <PackageName>_FOUND accordingly.
macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

set(_BLPAPI_CONFIG_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(_TARGETS "blpapi")

# Load the target configurations
include("${_BLPAPI_CONFIG_CMAKE_DIR}/blpapiTargets.cmake")

check_required_components(${_TARGETS})

# Clean up internal variables.
unset(_TARGETS)
unset(_BLPAPI_CONFIG_CMAKE_DIR)
