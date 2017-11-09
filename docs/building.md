CMake
=====
> If you are already familiar with cmake skip to your platform's building instructions.

1. Download and install [CMake](https://cmake.org/download/).
2. Add CMake to your path if the installer didn't add it for you.

Or us your package manager ..


Building
========

Windows
-------

Install a real operating system.
https://distrowatch.com/


Mac
-------

Install a real operating system.
https://distrowatch.com/


Linux
-----

To build on Linux generate makefiles with CMake.

1. Make sure a recursive clone of the project is done to download all submodules.
2. Enter the working directory
3. Create a build directory for CMake artifacts.
4. Create makefiles
5. Build with make

```bash
git clone --recursive https://github.com/Patrick-Edouard/s2client-api
cd s2client-api
mkdir build
cd build
cmake ../
make
```

Compilation troubleshooting
=======================

I once issued an error while compiling due to a bad gcc-ar program on my machine that generated the following trace :

```
Scanning dependencies of target c-library
[  0%] Building C object contrib/civetweb/src/CMakeFiles/c-library.dir/civetweb.c.o
[  0%] Linking C static library ../../../bin/libcivetweb.a
Error running link command: Segmentation fault
```

It can be fixed by going into 

>/s2client-api/contrib/civetweb/CMakeLists.txt

And commentating the block referencing gcc-ar/

```
# Configure the linker
#if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
#  find_program(GCC_AR gcc-ar)
#  if (GCC_AR)
#    set(CMAKE_AR ${GCC_AR})
#  endif()
#  find_program(GCC_RANLIB gcc-ranlib)
#  if (GCC_RANLIB)
#    set(CMAKE_RANLIB ${GCC_RANLIB})
#  endif()
#endif()

```




Installing game textures
=======================

For now, this headful version requiere external textures to be added. We need one image file for each unit. since orientation and annimation are yet to be implemented, a plain old .png image is enough. For instance there is a pack of images availiable here . https://www.reddit.com/r/starcraft/comments/1axm20/hires_starcraft_2_units_structures_pngs_with/

You need to rename the images to match the naming convention so they are dynamically used by the renderer using the following pattern/

```
RACE_UNITNAME

TERRAN_SCV
```

The folder is defined in the variable texture_forlder of sc2_rendere.cc


Working with submodules
=======================

If you cloned the repo without specifying --recursive you can initialize and checkout
the required submodules with the following git command -

```bash
git submodule update --init --recursive
```


Launching the human controller
=======================

An executable named humanController should be produced. It requieres a path to the headless sc2 client as an argument.

```bash
./humanController -e /path/to/sc2/healess
```
