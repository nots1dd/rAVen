#!/bin/bash
# Recompile the dynamic library

clang -v -fPIC -shared ../lib/libraven.c -o ../lib/libraven.so \
    `pkg-config --cflags --libs raylib gtk+-3.0 libavformat libavutil Magick++` \
    -lglfw -lmagic -lraylib -ldl
gcc ../src/main.c -ldl -o ../build/rAVenDync -DEASC_DYNC `pkg-config --cflags --libs raylib gtk+-3.0 libavformat libavutil Magick++` -lglfw -lmagic -lraylib
if [ $? -eq 0 ]; then
    echo -e "\033[1;32mLibrary recompiled successfully.\033[0m"
else
    echo -e "\033[1;31mError recompiling the library.\033[0m"
    exit 1
fi
