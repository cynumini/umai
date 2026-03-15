#!/usr/bin/env sh

set -e

warnings="-Wall -Wpedantic -Wextra -Werror"
libs="-lraylib -lsqlite3"
flags="$warnings $libs -g -std=c++20"
src="./src/experiment.cpp ./src/experiment-ui.cpp"
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
