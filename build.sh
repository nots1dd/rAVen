#!/bin/bash 

set -xe

CFLAGS="-Wall -Wextra -Wpedantic `pkg-config --cflags raylib`"
LIBS="`pkg-config --libs raylib` -lglfw -lm -ldl -pthread"

clang $CFLAGS -o raven main.c $LIBS 
