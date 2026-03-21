#!/usr/bin/env sh

set -e

flags="-Wall -Wextra -Wpedantic -Werror -Wconversion -std=c23"
echo $flags | tr ' ' "\n" > ./compile_flags.txt
flags="$flags -lraylib"
gcc main.c -o umai $flags
./umai
