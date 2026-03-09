#!/usr/bin/env sh

set -e

warnings="-Wall -Wpedantic -Wextra -Werror"
libs="-lraylib"
flags="$warnings $libs -g -std=c++17"
src="./src/main.cpp"
bin="./out/umai"

mkdir -p out
echo $flags | tr ' ' "\n" > ./compile_flags.txt
echo "#### build ####"
g++ $src -o $bin $flags
if [ "$1" = "gf2" ]; then
	echo "#### debug ####"
	gf2 --args $bin
else
	echo "#### run ####"
	$bin
fi
