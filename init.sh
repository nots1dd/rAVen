#!/bin/bash

CONFIG_FILE="/home/s1dd/misc/av/rAVenDync/.config/easC/config.json"

if [ ! -f "$CONFIG_FILE" ]; then
    echo "\033[1;31mConfig file not found.\033[0m"
    exit 1
fi

output_binary=$(jq -r '.output_binary' "$CONFIG_FILE")
library_name=$(jq -r '.library_name' "$CONFIG_FILE")

# Compile library and main program
clang -v -fPIC -shared lib/$library_name.c -o lib/$library_name.so \
    `pkg-config --cflags --libs raylib gtk+-3.0 libavformat libavutil Magick++` \
    -lglfw -lmagic -lraylib -ldl
if [ $? -eq 0 ]; then
  echo -e "\033[1;32mDynamic lib compiled successfully.\033[0m"
else 
  echo -e "\033[1;31mCompilation of dynamic lib failed.\033[0m"
  exit 1
fi 

gcc src/main.c -ldl -o build/$output_binary -DEASC_DYNC `pkg-config --cflags --libs raylib gtk+-3.0 libavformat libavutil Magick++` -lglfw -lmagic -lraylib
if [ $? -eq 0 ]; then
    echo -e "\033[1;32mProject compiled successfully.\033[0m"
else
    echo -e "\033[1;31mCompilation failed.\033[0m"
fi
