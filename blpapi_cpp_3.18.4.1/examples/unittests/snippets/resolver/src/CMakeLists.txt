add_library(resolverUtilObjects OBJECT resolver_utils.cpp)
target_include_directories(resolverUtilObjects
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(resolverUtilObjects PUBLIC blpapi)

add_executable(resolverUtil main.cpp)
target_link_libraries(resolverUtil PUBLIC resolverUtilObjects)
