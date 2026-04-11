#!/usr/bin/env sh

set -e

flags="-Wall -Wpedantic -Wextra -Werror -Wconversion -std=c23"

gcc main.c -o umai -lraylib $flags

./umai
