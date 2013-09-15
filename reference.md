## Lexical Syntax

Anything beginning with `;` or `#` denotes a comment. If the tokens are placed anywhere other than the beginning, then the contents are not commented out. Shinobi has multiple variables to configure the output of the generated file. These are case-sensitive and can be modified by two operators. The first operator is `:=`, which assigns and overrides the contents. The second is `+=` which appends and doesn't override the contents of the variable. Variables follow the following syntax:

    VARIABLE_NAME operator VALUE


If a given variable is missing, a specified sensible default value is provided instead.

### Platform Dependent Assignment

Sometimes, we want to have certain compile-flags be platform dependent or even something else entirely. Shinobi supports this by the use of an if statement. For example, suppose we wanted to add `NOMINMAX` if Windows is the current platform. We could express this as follows:

    # Set up defines for all operating systems
    DEFINES := -DNDEBUG

    # Other info...

    # Windows specific code
    if Windows
    DEFINES += -DNOMINMAX
    endif


The syntax is pretty straight forward. You start an if block with the `if` statement followed by a space and then the operating system you're targetting, then the variables that need modifying on a newline and finish it up with the `endif` statement. Current supported strings are `Windows`, `Linux`, and `MacOS`. If neither of those are found then the platform is called `Other`. These are all case sensitive. 

To reiterate, the syntax is as follows:

    if OPERATING_SYSTEM
    EXPRESSIONS
    endif


## Available Variables

### BUILDDIR

**Default value:** `bin`

The directory where the resulting executable and ninja log files are dropped to. This variable shouldn't have a leading / or \ as that will probably mess up the resulting output file. If the directory contains spaces, quotes should be used around the directory. It makes no sense to have multiple values for this variable.

### PROJECT_NAME

**Default value:** `untitled`

The resulting name of the executable. The resulting executable is found under the `BUILDDIR` directory.

### CXX

**Default value:** `g++`

The compiler executable being used. The compiler could theoretically be `gcc`, `g++`, `clang`, `clang++` or any of those variants that support their compiler flags and syntax.

### CXXFLAGS

**Default value:** `-std=c++11 -pedantic -pedantic-errors -Wextra -Wall -O2`

The compiler flags being passed to the compiler. These are passed during the *compiling* phase, not the linking phase. 

### DEFINES

**Default value:** `-DNDEBUG`

The preprocessor directives to be defined by the compiler. It is the equivalent to `#define STUFF`. Following that example, the resulting define would be `-DSTUFF`.

### INCLUDE_FLAGS

**Default value:** `-I.`

The directories the compiler should look for when using `#include` or any similar directive. These are usually prefixed with `-I`, e.g. `-I"directory"`.

### LIBRARY_FLAGS

**No default value**

The flags to be passed to the linker during linking time. The order is usually important. These are prefixed with `-l` as a shortcut to `libname.a`. So following that example, the resulting flag would be `-lname`.

### LIBRARY_PATHS

**No default value**

The directories the compiler should look for to find libraries. These are usually prefixed with `-L`, e.g. `-L"directory"`. 

### OBJDIR

**Default value:** `obj`

Similar to `BUILDDIR`, the directory where the resulting .o files are stored. This variable shouldn't have a leading / or \ as that will probably mess up the resulting output file. If the directory contains spaces, quotes should be used around the directory. It makes no sense to have multiple values for this variable.

### SRCDIR

**Default value:** `./`

The directory that is recursively searched to find `.cpp`, `.cxx`, `.cc`, `.c++`, and `.c` files for building. The resulting files are then built separately and linked together to an executable whose name is given by the `PROJECT_NAME` variable. By default, `shinobi` looks for every file that meets the requirements specified earlier. If the directory contains spaces, quotes should be used around the directory. Currently only one directory is supported, though more directories will be supported soon.