#!/bin/bash 

set -xe

CFLAGS="-Wall -Wextra -Wpedantic `pkg-config --cflags raylib` `pkg-config --cflags gtk+-3.0`"
LIBS="`pkg-config --libs raylib` -lglfw -lm -ldl -pthread `pkg-config --libs gtk+-3.0`"

clang $CFLAGS -o raven main.c $LIBS 
