add_executable(eventtests events.t.cpp)
target_link_libraries(eventtests PUBLIC blpapi gtest gmock)

gtest_add_tests(TARGET eventtests)
