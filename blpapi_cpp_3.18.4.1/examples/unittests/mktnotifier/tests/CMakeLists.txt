add_executable(mktnotifiertests
  "application.t.cpp"
  "authorizer.t.cpp"
  "eventprocessor.t.cpp"
  "test.t.cpp"
  "testSchemas.cpp"
  "tokengenerator.t.cpp")

target_link_libraries(mktnotifiertests PUBLIC
  mktnotifiersobjects
  blpapi
  gtest
  gmock
  "${CMAKE_THREAD_LIBS_INIT}")

gtest_add_tests(TARGET mktnotifiertests)
