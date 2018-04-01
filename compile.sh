#!/bin/bash

# Causes the script to exit if any command returns non zero
set -e

filename="${1%.*}"

#Debug version prints llvm IR to stderr
./bin/compiler $1 2> "${filename}.ll"

clang "${filename}.ll" ./src/runtime/runtime.c -o "${filename##*/}.out" -g -O3

#Remove intermediates
rm "${filename}.ll"

