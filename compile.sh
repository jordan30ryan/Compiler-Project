#!/bin/bash

# Causes the script to exit if any command returns non zero
set -e

filename="${1%.*}"
outfile="${filename##*/}.out"

./bin/compiler $1
#opt -S -O3 "${filename}.ll" -o "${filename}.ll"
llc "${filename}.ll"
clang "${filename}.s" ./src/runtime/runtime.c -o $outfile

#Remove intermediates
rm "${filename}.ll"
rm "${filename}.s"

