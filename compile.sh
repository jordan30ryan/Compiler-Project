#!/bin/bash

# Causes the script to exit if any command returns non zero
set -e

filename="${1%.*}"

./bin/compiler $1

#Ensure llvm > 3.5 is used
#llc "${filename}.ll"

clang "${filename}.s" ./src/runtime/runtime.c -o "${filename##*/}.out"

#Remove intermediates
#rm "${filename}.ll"
rm "${filename}.s"

