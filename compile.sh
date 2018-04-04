#!/bin/sh

# Causes the script to exit if any command returns non zero
set -e

filename="${1%.*}"

#Write out .ll file from .src file
./bin/compiler $1 

#Compile/link the .ll file and runtime
clang "${filename}.ll" ./src/runtime/runtime.c -o "${filename##*/}.out" -g -O3

#Remove intermediates
rm "${filename}.ll"

