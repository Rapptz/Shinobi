# shinobi

`shinobi` is a meta build system for [Ninja][1]. It uses an INI-file like structure to configure the output of the build.ninja file. `shinobi` implicitly creates a `bin/` and `obj/` directory to circumvent the issue Ninja has with building on non-existing directories. `shinobi` works by pulling in all `.cpp`, `.cxx`, `.cc`, and `.c++` files and making them into basic build commands. There is also support for C by allowing `.c` extensions.

Currently only `clang++` and `g++` are supported. Of course, `gcc` and `clang` are as well since they follow similar syntax. Technically anything with similar syntax to `g++` should run, despite the variable names. There might be more customisation and support for MSVC in the future, however this isn't currently planned.

[1]: http://martine.github.io/ninja/

# Shinobi File

In order to run `shinobi`, you need a Shinobi file without any extension (note the uppercase S). This file has specific variable names and parsing rules that allow the creation of the build.ninja file. By default, invoking `shinobi` without an existing Shinobi file will lead to a default one being created with the following material:

    PROJECT_NAME := untitled
    CXX := g++
    CXXFLAGS += -std=c++11 -pedantic -pedantic-errors -Wextra -Wall -O2
    INCLUDE_FLAGS += -I.
    LIBRARY_PATHS :=
    LIBRARY_FLAGS :=
    DEFINES += -DNDEBUG


We will go over the syntax soon, just know currently that you can append with `+=` and assign with `:=`. Assigning overrides anything, while appending obviously doesn't.

## Syntax

Currently the Shinobi file uses specific case-sensitive variable names to control its output since there isn't much variation in those. These are:

- `PROJECT_NAME`: The resulting name of the executable. The resulting executable is found under the `bin/` directory. Default value is `untitled`
- `CXX`: The compiler executable being used. Default value is `g++`
- `CXXFLAGS`: The compiler flags being passed to the compiler. The default flags are `-std=c++11 -pedantic -pedantic-errors -Wextra -Wall -O2`
- `INCLUDE_FLAGS`: The directories the compiler should look for when using `#include` or any similar directive. These are usually prefixed with `-I`, e.g. `-I"directory"`. The default value is `-I.`.
- `LIBRARY_PATHS`: The directories the compiler should look for to find libraries. These are usually prefixed with `-L`, e.g. `-L"directory"`. There is no default value.
- `LIBRARY_FLAGS`: The flags to be passed to the linker during linking time. The order is usually important. These are prefixed with `-l` as a shortcut to `libname.a`. So following that example, the resulting flag would be `-lname`. There is no default value.
- `DEFINES`: The preprocessor directives to be defined by the compiler. It is the equivalent to `#define STUFF`. Following that example, the resulting define would be `-DSTUFF`. The default value is `-DNDEBUG`.

Along with those variables, `;` and `#` in the beginning of a line denotes a comment. Anything after that comment is ignored until the next non-commented line.

## Example files

### Shinobi

    PROJECT_NAME := shinobi
    CXX := g++
    CXXFLAGS += -std=c++11 -pedantic -pedantic-errors -Wextra -Wall -O2
    INCLUDE_FLAGS += -I.
    LIBRARY_PATHS +=
    LIBRARY_FLAGS += -lboost_system -lboost_filesystem
    DEFINES += -DNDEBUG


### build.ninja

    cxx = g++

    cxxflags = -std=c++11 -pedantic -pedantic-errors -Wextra -Wall -O2

    incflags = -I.

    libpath = 

    lib = -lboost_system -lboost_filesystem

    def = -DNDEBUG

    rule bd
        command = ${cxx} ${cxxflags} ${def} -c ${in} -o ${out} ${incflags}

    rule ld
        command = ${cxx} ${in} -o ${out} ${libpath} ${lib}

    build obj\shinobi.o: bd shinobi.cpp

    build bin\shinobi: ld obj\shinobi.o


These are actually the files used to build `shinobi` itself. It assumes Boost libraries are in the system directories.

# Compiling shinobi

`shinobi` requires any recent compiler using C++11 and variadic arguments. Boost.Filesystem is also required for the cross-platform file handling. 