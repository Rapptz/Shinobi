# shinobi

`shinobi` is a meta build system for [Ninja][1]. It uses an [qmake][2]-like structure to configure the output of the build.ninja file. `shinobi` implicitly creates a `bin/` and `obj/` directory to circumvent the issue Ninja has with building on non-existing directories. `shinobi` works by pulling in all `.cpp`, `.cxx`, `.cc`, and `.c++` files and making them into basic build commands. There is also support for C by allowing `.c` extensions. The generated build.ninja requires ninja v1.3 or higher due to usage of `deps`.

Currently only `clang++` and `g++` are supported. Of course, `gcc` and `clang` are as well since they follow similar syntax. Technically anything with similar syntax to `g++` should run, despite the variable names. There might be more customisation and support for MSVC in the future, however this isn't currently planned.

[1]: http://martine.github.io/ninja/
[2]: https://qt-project.org/doc/qt-4.8/qmake-manual.html

# Shinobi File

In order to run `shinobi`, you need a Shinobi file without any extension (note the uppercase S). This file has specific variable names and parsing rules that allow the creation of the build.ninja file. By default, invoking `shinobi` without an existing Shinobi file will lead to a default one being created with the following material:

    PROJECT_NAME := untitled
    BUILDDIR := bin
    OBJDIR := obj
    CXX := g++
    CXXFLAGS += -std=c++11 -pedantic -pedantic-errors -Wextra -Wall -O2
    INCLUDE_FLAGS += -I.
    LIBRARY_PATHS +=
    LIBRARY_FLAGS +=
    DEFINES += -DNDEBUG

Reference for the Shinobi file can be found in reference.md

# Example Files

## Shinobi

    PROJECT_NAME := shinobi
    CXX := g++
    CXXFLAGS += -std=c++11 -pedantic -pedantic-errors -Wextra -Wall -O2
    INCLUDE_FLAGS += -I.
    LIBRARY_FLAGS += -static -lboost_system -lboost_filesystem
    DEFINES += -DNDEBUG


## build.ninja

    ninja_required_version = 1.3
    builddir = bin
    objdir = obj
    cxx = g++
    cxxflags = -std=c++11 -pedantic -pedantic-errors -Wextra -Wall -O2
    incflags = -I.
    libpath = 
    lib = -static -lboost_system -lboost_filesystem
    def = -DNDEBUG

    rule compile
        deps = gcc
        depfile = $out.d
        command = $cxx -MMD -MT $out -MF $out.d $cxxflags $def -c $in -o $out $incflags
        description = Building $in to $out

    rule link
        command = $cxx $in -o $out $libpath $lib
        description = Linking $out

    build $objdir/shinobi.o: compile shinobi.cpp

    build $builddir/shinobi: link $objdir/shinobi.o


These are actually the files used to build `shinobi` itself. It assumes Boost libraries are in the system directories.

# Compiling shinobi

`shinobi` requires any recent compiler using C++11 and variadic templates. Boost.Filesystem is also required for the cross-platform file handling. By theory any GCC over 4.7 and Clang over 3.1 should work. VS2013 might work also.