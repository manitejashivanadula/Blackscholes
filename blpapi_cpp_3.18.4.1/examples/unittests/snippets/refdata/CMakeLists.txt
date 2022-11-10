add_executable(refdatatests refdata.t.cpp)
target_include_directories(refdatatests PUBLIC blpapi)
target_link_libraries(refdatatests PUBLIC blpapi gtest gmock)

gtest_add_tests(TARGET refdatatests)
