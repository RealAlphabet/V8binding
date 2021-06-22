#!/bin/bash

export LD_LIBRARY_PATH=$(realpath ../)

# Compile test.
gcc main.c -O3 -I../include -L../ -lv8c -g

# Execute test.
valgrind ./a.out