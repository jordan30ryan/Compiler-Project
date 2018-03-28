# Super basic makefile
c: CC=clang++
c: CFLAGS=-Wall -std=c++11 -Wno-char-subscripts `llvm-config --cxxflags --ldflags --system-libs --libs core` -ggdb

g: CC=g++
g: CFLAGS=-Wall --std=c++11 -Wno-char-subscripts -ggdb

c2: CC=clang++
c2: CFLAGS=-Wall -std=c++11 -Wno-char-subscripts -stdlib=libc++

c: ./src/*.cpp
	@ mkdir -p bin
	$(CC) $(CFLAGS) -o ./bin/compiler ./src/*.cpp

g: ./src/*.cpp
	@ mkdir -p bin
	$(CC) $(CFLAGS) -o ./bin/compiler ./src/*.cpp

c2: ./src/*.cpp
	@ mkdir -p bin
	$(CC) $(CFLAGS) -o ./bin/compiler ./src/*.cpp
