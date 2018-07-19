# Compiler-project

EECE 5183 Project

## Dependencies

clang (5.0 or above)
llvm and llvm-config (5.0 or above)

## BUILD

With clang++ (at least 5.0):

    make


If clang/llvm are installed as clang-5.0/llvm-5.0 
    (this seems to be default for apt):

    make compiler-c5

## USAGE

Compile with:

    ./compile.sh <input_file>.src

Run with:

    ./<input_file>.out
