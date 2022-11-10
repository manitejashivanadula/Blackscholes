# Blpapi testing support

`TestUtil` provides users with ability to test their applications offline
through the creation of custom events for their applications.

We are providing in this directory a simple application `mktnotifier`
complete with tests.

Together with the example application we are providing code `snippets` that can
be used in your own tests and demonstrate the construction of some common
messages and events.

We have chosen to use `cmake` as the build system and `googletest` for our
tests, but the code can easily be ported to a different build system or
test framework.

## Building and running

The application and associated test drivers can be built and run following
these steps:

1. `mkdir build`
2. `cd build`
3. `cmake ..`

If you don't have access to internet you can provide the path to googletest
source directory with the following command:

`cmake -DGTEST_SRC_DIR=<path to google testing framework src> ..`

4. `cmake --build . --config Release`. By default test cases are built in `Release` configuration.
5. `ctest` for platform other than Windows. For Windows use `ctest -C Release`

Configuration can be changed by passing `-DCMAKE_BUILD_TYPE=<desired-config>`
in step 3., and `--config <desired-config>` in step 4. Same configuration needs
to be passed to `ctest` in step 5.
