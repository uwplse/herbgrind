#!/bin/env bash

# Comment out uses of malloc, realloc, free, and fprinf, in the
# default memory functions for gmp. We'll be calling
# mp_set_memory_functions to override the default memory functions in
# our tool anyway, so the default ones wil never be called. But the
# linker will still try and link them, and since valgrind doesn't
# provide a standard c library to it's tools, it will fail, killing
# the build. So, we comment them out instead, and everything works
# fine.
sed -i \
    -e 's/malloc.*(.*);/0;\/\/&/' \
    -e 's/realloc.*(.*);/0;\/\/&/' \
    -e 's/free.*(.*);/\/\/&/' \
    -e 's/fprintf.*(.*);/\/\/&/'\
    ../deps/gmp-$1/memory.c
