#!/bin/bash

filename="${1%.*}"
llc "${filename}.ll"
clang "${filename}.s" ./src/runtime/runtime.c

