# Building
## Install dependencies
The `install_deps` script will install the appropriate dependancies to build the entire project. However, this script is only supported for Ubuntu 12.04 as that is the architecture that will be producing the release binary. `install_deps` should be easy to adapt to other systems for development purposes, and I encourage you to just look at the `install_dups` script when trying to figure out what you need for your system.

## One-step build
To build the resulting binary, just run `./one_step_build`. When building, everything will be put in a `build/` directory. If you want to build a debug build, do `./one_step_build debug`.

## Modifying the build process
### CMake
We use CMake for building. There are resources online, but in general it's usage should be straight forward from the existing build files.

### CMakeLists.txt
CMake uses a file called CMakeLists.txt to specify how everything should be built. This file contains both the compiler configuration while building and the things to build. When running CMake on a directory, CMake will find the CMakeLists.txt and use it to generate the Makefile, among other things.

### add\_unit\_test
Use the `add_unit_test` macro to add the unit test to the testing suite. It's usage should be clear from the existing tests. `make check` will run all of the tests, this is done by the `run_tests` script.
