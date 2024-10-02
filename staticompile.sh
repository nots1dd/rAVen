#!/bin/bash

YELLOW='\033[1;33m'
RED='\033[1;31m'
GREEN='\033[1;32m'
BLUE='\033[1;34m'
NC='\033[0m' # No Color

CONFIG_FILE="/home/s1dd/misc/av/rAVen/rAVenDync/.config/easC/config.json"

if [ ! -f "$CONFIG_FILE" ]; then
    echo "${RED}Config file not found.${NC}"
    exit 1
fi

output_binary=$(jq -r '.output_binary' "$CONFIG_FILE")-static
library_name=$(jq -r '.library_name' "$CONFIG_FILE")

# Compile library and main program
gcc -fPIC -shared lib/$library_name.c -o lib/$library_name.so
gcc lib/$library_name.c src/main.c -ldl -o build/$output_binary
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Project compiled successfully.${NC}"
else
    echo -e "${RED}Compilation failed.${NC}"
fi
