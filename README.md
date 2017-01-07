clog: Extremely lightweight C logger
====================================

clog is a logging library for C/C++ programs, implemented as a single header
file.  It is meant for simple utilities and programs where introducing new
dependencies causes a lot of trouble.  The only thing clog depends on is a
POSIX environment.

**Features:**

* Implemented as a single header file.
* C99 / C++98 conformance.\*
* Multiple loggers (numbered: 0 - 15).
* Four severity levels (debug, info, warn, error).
* Customizable log format, time format, date format.
* Relatively fast (real world 180k logs/sec on my laptop).
* Log to an arbitrary file descriptor (socket, pipe, etc).
* No licensing restrictions whatsoever.

It may be useful for embedded environments, but it has not been written
specifically for embedded use.  As of ~January 2013, clog is basically brand
new and I welcome feature suggestions and patches from other developers.  It
uses the most widely-available POSIX I/O functions and attempts to avoid
anything non-standard.

**Missing features:**

* Millisecond/microsecond timestamps (will require OS-specific patches).
* Support custom allocators (rather than malloc).
* Variadic macros, because those are not C++98 compatible. (Considering adding
  anyway soon.)

\* Requires `vsnprintf()` and `va_copy()` to exist. These might not be
   available on every C++98 compiler, so please let me know if you run into a
   compiler that complains. May work on some C89 compilers, but I'm not
   supporting C89 officially.

Download
--------

Get `clog.h` here:
https://raw.github.com/mmueller/clog/master/clog.h

You do not need the rest of this repository unless you wish to run the tests
or develop clog yourself.

Usage
-----

1. Download the latest version of `clog.h` and drop it somewhere in your
   project.
2. Include the file anywhere you wish to use loggers.
3. In exactly one file per executable, `#define CLOG_MAIN` before including
   it.  This is necessary to link the clog function implementations into your
   program.

Example:

    #include <stdio.h>
    
    #define CLOG_MAIN
    #include "clog.h"
    
    const int MY_LOGGER = 0; /* Unique identifier for logger */
    
    int main() {
        int r;

        /* Initialize the logger */
        r = clog_init_path(MY_LOGGER, "my_log.txt");
        if (r != 0) {
            fprintf(stderr, "Logger initialization failed.\n");
            return 1;
        }

        /* Set minimum log level to info (default: debug) */
        clog_set_level(MY_LOGGER, CLOG_INFO);

        /* Write a message */
        clog_info(CLOG(MY_LOGGER), "Hello, %s!", "world");

        /* Clean up */
        clog_free(MY_LOGGER);
        return 0;
    }

This example appends a single line to the file `my_log.txt`:

    2012-12-29 15:34:27 example.c(16): INFO: Hello, world!

Note the use of the `CLOG()` macro in the call to `clog_info()`.  This is a
convenience macro that passes `__FILE__, __LINE__, id` to the logging function
(so it can write the source file and line number to the log).  It's just there
to save you a little typing.  Ideally `clog_info()` would be a variadic macro
and there would be no `CLOG()` macro, but this would be outside of the realms
of ISO C89 and C++98.

See `clog.h` for full API documentation.
