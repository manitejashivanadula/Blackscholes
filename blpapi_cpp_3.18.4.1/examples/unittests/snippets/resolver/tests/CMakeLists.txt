add_executable(providerTests resolver_utils.t.cpp)
target_link_libraries(providerTests PUBLIC resolverUtilObjects gtest gmock)

gtest_add_tests(TARGET providerTests)
