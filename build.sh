#!/usr/bin/env sh

set -e

python templates.py 

flags="-Wall -Wextra -Wpedantic -Werror -Wconversion -std=c23"
echo $flags | tr ' ' "\n" > ./compile_flags.txt
flags="$flags -lraylib -g"
gcc main.c ui.c optional.c math.c paddings.c -o umai $flags
./umai
