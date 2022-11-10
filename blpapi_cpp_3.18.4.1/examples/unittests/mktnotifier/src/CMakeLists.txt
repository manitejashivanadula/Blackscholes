set(_SOURCES
    "appconfig.cpp"
    "application.cpp"
    "authorizer.cpp"
    "computeengine.cpp"
    "eventprocessor.cpp"
    "notifier.cpp"
    "subscriber.cpp"
    "tokengenerator.cpp")

add_library(mktnotifiersobjects OBJECT "${_SOURCES}")
target_include_directories(mktnotifiersobjects
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(mktnotifiersobjects PUBLIC blpapi)

add_executable(mktnotifier main.cpp)
target_link_libraries(mktnotifier PUBLIC mktnotifiersobjects)
