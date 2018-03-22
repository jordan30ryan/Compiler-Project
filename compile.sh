#!/bin/bash

filename="${1%.*}"
outfile="${filename##*/}.out"

./bin/compiler $1
llc "${filename}.ll"
clang "${filename}.s" ./src/runtime/runtime.c -o $outfile

#Remove intermediates
rm "${filename}.ll"
rm "${filename}.s"

