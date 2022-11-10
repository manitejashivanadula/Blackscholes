#------------------------------------------------------------------------------
# CMake target import file for configuration Release.
#------------------------------------------------------------------------------

# Import targets for configuration "Release".

# Path to blpapi shared object in relation to blpapiConfig.cmake:
# 1. On Windows
#    a. For release ../lib/
# 2. On other platforms
#    a. For release ../<SystemName>/. For example for linux it is ../Linux/

# Windows
if(WIN32)
  set(_BLPAPI_SHARED_OBJ_NAME "${_BLPAPI_PREFIX}_${_ARCH}.dll")
  set(_BLPAPI_SHARED_IMP_OBJ_NAME "${_BLPAPI_PREFIX}_${_ARCH}.lib")
  set(_BLPAPI_SHARED_OBJ "${_BLPAPI_CONFIG_CMAKE_DIR}/../lib/${_BLPAPI_SHARED_OBJ_NAME}")
  set(_BLPAPI_IMP_OBJ "${_BLPAPI_CONFIG_CMAKE_DIR}/../lib/${_BLPAPI_SHARED_IMP_OBJ_NAME}")
else() # Other platforms
  set(_BLPAPI_SHARED_OBJ_NAME "lib${_BLPAPI_PREFIX}_${_ARCH}.so")
  set(_BLPAPI_SHARED_OBJ
      "${_BLPAPI_CONFIG_CMAKE_DIR}/../${CMAKE_SYSTEM_NAME}/${_BLPAPI_SHARED_OBJ_NAME}")
endif()

set_property(TARGET blpapi APPEND PROPERTY IMPORTED_LOCATION "${_BLPAPI_SHARED_OBJ}")
list(APPEND _IMPORT_CHECK_FILES_FOR_blpapi "${_BLPAPI_SHARED_OBJ}")

if(WIN32)
  set_property(TARGET blpapi APPEND PROPERTY IMPORTED_IMPLIB "${_BLPAPI_IMP_OBJ}")
  list(APPEND _IMPORT_CHECK_FILES_FOR_blpapi "${_BLPAPI_IMP_OBJ}")
endif()

unset(_BLPAPI_IMP_OBJ)
unset(_BLPAPI_SHARED_OBJ)
unset(_BLPAPI_SHARED_OBJ_NAME)
unset(_BLPAPI_SHARED_IMP_OBJ_NAME)
