#!/bin/bash
# Recompile the dynamic library

clang -v -fPIC -shared libraven.c -o libraven.so \
    `pkg-config --cflags --libs raylib gtk+-3.0 libavformat libavutil Magick++` \
    -lglfw -lmagic -lraylib -ldl
if [ $? -eq 0 ]; then
    echo -e "\033[1;32mLibrary recompiled successfully.\033[0m"
else
    echo -e "\033[1;31mError recompiling the library.\033[0m"
    exit 1
fi
