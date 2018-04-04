# Super basic makefile
c: CC=clang++
c: CFLAGS=-Wall -std=c++11 `llvm-config --cxxflags --ldflags --system-libs --libs core` -ggdb -Wno-unknown-warning-option -O3

c: ./src/*.cpp
	@ mkdir -p bin
	$(CC) $(CFLAGS) -o ./bin/compiler ./src/*.cpp

