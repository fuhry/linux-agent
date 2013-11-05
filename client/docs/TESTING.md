# Testing

## Google Test
Google provides an extremely easy to use and feature complete testing framework. There is good documentation available, in particular the [Primer](https://code.google.com/p/googletest/wiki/Primer).

### Installing
Google test is automatically checked out as part of the build process. See `cmake/test_setup.cmake` for details.

## Unit Testing
Here's a [quick write-up](http://readwrite.com/2008/08/13/12_unit_testing_tips_for_software_engineers) on unit testing. Worth a skim even if you are familiar with unit testing.

## Test directory
All tests should be in the `test/` directory. Common components for these tests should also be in `test/`. Tests should be named after the class they are testing with the prefix `_test`.

## What needs automated tests
All components should have unit tests. Note that this doesn't mean all classes, but the primary interface of a component should be automatically tested. If unsure if something should be tested, ask. Some unit tests will be straight forward (see unsynced\_sector\_store\_test), and some will be more complicated (see device\_tracer\_test).

## Running tests when executing run\_tests
Update the CMakeLists.txt file to add your test to the test suite. How to do this should be obvious from context.

## Scripts during testing
While we should avoid shelling out as much as possible, somethings are much easier to do with a bash script than in C++. See device\_tracer\_test for an example of how to use scripts to setup the environment.
