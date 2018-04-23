# Super basic makefile

compiler: CC=clang++
compiler: CFLAGS=-Wall -std=c++11 `llvm-config --cxxflags --ldflags --system-libs --libs core` -Wno-unknown-warning-option -O0 -g

compiler: ./src/*.cpp
	@ mkdir -p bin
	$(CC) $(CFLAGS) -o ./bin/compiler ./src/*.cpp


compiler-c5: CC=clang++-5.0
compiler-c5: CFLAGS=-Wall -std=c++11 `llvm-config-5.0 --cxxflags --ldflags --system-libs --libs core` -Wno-unknown-warning-option -O3

compiler-c5: ./src/*.cpp
	@ mkdir -p bin
	$(CC) $(CFLAGS) -o ./bin/compiler ./src/*.cpp
