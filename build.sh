#!/bin/bash 

set -xe

CFLAGS="-Wall -Wextra -Wpedantic `pkg-config --cflags raylib` `pkg-config --cflags gtk+-3.0` `pkg-config --cflags libavformat`"
LIBS="`pkg-config --libs raylib` -lglfw -lm -ldl -lpthread `pkg-config --libs libavformat` `pkg-config --libs gtk+-3.0` `pkg-config --libs libavutil`"

clang -v $CFLAGS -o raven main.c $LIBS 
