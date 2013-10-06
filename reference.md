## Lexical Syntax

Anything beginning with `;` or `#` denotes a comment. If the tokens are placed anywhere other than the beginning, then the 
contents are not commented out. Shinobi has multiple variables to configure the output of the generated file. These are 
case-sensitive and can be modified by two operators. Shinobi has multiple operators and they all do something different. 
They can be found in the table below.


| Operator | Description                              |
|:--------:|:----------------------------------------:|
| +=       | Appends value to variable.               |
| :=       | Assigns and overrides value to variable. |
| -=       | Erases value from variable.              |


To reiterate the syntax:


    VARIABLE_NAME operator VALUE


Where VARIABLE_NAME is one of the pre-set variables, the operator is found in the table above and VALUE is, well, the value.


If a given variable is missing, a specified sensible default value is provided instead. Variable assignment must be put on a 
single line, so the following won't work:

    DEFINES += -DNDEBUG \
               -DNOMINMAX \
               -DSTUFF


instead use the following:

    DEFINES += -DNDEBUG -DNOMINMAX -DSTUFF


### Platform Dependent Assignment

Sometimes, we want to have certain compile-flags be platform dependent or even something else entirely. Shinobi supports 
this by the use of an if statement. For example, suppose we wanted to add `NOMINMAX` if Windows is the current platform. 
We could express this as follows:

    # Set up defines for all operating systems
    DEFINES := -DNDEBUG

    # Other info...

    # Windows specific code
    if(Windows)
    DEFINES += -DNOMINMAX
    endif

The syntax for an if-statement is simple, it's similar to those found in C-like languages in which the boolean expression is 
enclosed in parentheses. Certain boolean variables are supported and will be listed below. They are all case insensitive.

| Variable | Description                                                            |
|:--------:|:----------------------------------------------------------------------:|
| Windows  | Shinobi is ran in the Windows environment.                             |
| MacOS    | Shinobi is ran in the MacOS environment.                               |
| Linux    | Shinobi is ran in the Linux environment.                               |
| Debug    | The `-d` or `--debug` flags are passed to Shinobi.                     |
| Release  | The `-r` or `--release` flags are passed to Shinobi or Shinobi is ran. |


To reiterate, the syntax is as follows:

    if(BOOLEAN_VARIABLE)
    EXPRESSIONS
    endif


## Available Variables

### BUILDDIR

**Default value:** `bin`

The directory where the resulting executable and ninja log files are dropped to. This variable shouldn't have a leading / 
or \ as that will probably mess up the resulting output file. If the directory contains spaces, quotes should be used around 
the directory. It makes no sense to have multiple values for this variable.

### PROJECT_NAME

**Default value:** `untitled`

The resulting name of the executable. The resulting executable is found under the `BUILDDIR` directory.

### CXX

**Default value:** `g++`

The compiler executable being used. The compiler could theoretically be `gcc`, `g++`, `clang`, `clang++` or any of those 
variants that support their compiler flags and syntax.

### CXXFLAGS

**Default value:** `-std=c++11 -pedantic -pedantic-errors -Wextra -Wall -O2`

The compiler flags being passed to the compiler. These are passed during the *compiling* phase, not the linking phase. 

### DEFINES

**Default value:** `-DNDEBUG`

The preprocessor directives to be defined by the compiler. It is the equivalent to `#define STUFF`. Following that example, 
the resulting define would be `-DSTUFF`.

### IGNORED_FILES

**No default value**

The files that will be ignored. If the SRCDIR listed is `./`, then you must specify the ignored files without the relative 
path. Otherwise it must list the SRCDIR in the name, e.g. if SRCDIR is `src` then the ignored file must be `src/stuff.cpp`.
You must use forward-slashes (/) for paths and not back-slashes (\).

### INCLUDE_FLAGS

**Default value:** `-I.`

The directories the compiler should look for when using `#include` or any similar directive. These are usually prefixed 
with `-I`, e.g. `-I"directory"`.

### LIBS

**No default value**

The library flags to be passed to the linker during linking time. The order is usually important. These are prefixed with 
`-l` as a shortcut to `libname.a`. So following that example, the resulting flag would be `-lname`.

### LIB_PATHS

**No default value**

The directories the compiler should look for to find libraries. These are usually prefixed with `-L`, e.g. `-L"directory"`. 

### LINK_FLAGS

**Default value:** `-static`

The flags to be passed to the linker during linking time. Usually to set if something is statically linked or dynamically 
linked. These flags are passed before `LIB_PATHS` and `LIBS`.

### OBJDIR

**Default value:** `obj`

Similar to `BUILDDIR`, the directory where the resulting .o files are stored. This variable shouldn't have a leading / or \ 
as that will probably mess up the resulting output file. If the directory contains spaces, quotes should be used around the 
directory. It makes no sense to have multiple values for this variable.

### SRCDIR

**Default value:** `./`

The directory that is recursively searched to find `.cpp`, `.cxx`, `.cc`, `.c++`, and `.c` files for building. The resulting 
files are then built separately and linked together to an executable whose name is given by the `PROJECT_NAME` variable. By 
default, `shinobi` looks for every file that meets the requirements specified earlier. If the directory contains spaces, 
quotes should be used around the directory. Multiple directories are supported.