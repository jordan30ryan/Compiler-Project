# Super basic makefile
g: CC=g++
g: CFLAGS=-Wall --std=c++11 -Wno-char-subscripts 

c: CC=clang++
c: CFLAGS=-Wall -std=c++11 -Wno-char-subscripts -stdlib=libc++

g: ./src/*.cpp
	@ mkdir -p bin
	$(CC) $(CFLAGS) -o ./bin/compiler ./src/*.cpp

c: ./src/*.cpp
	@ mkdir -p bin
	$(CC) $(CFLAGS) -o ./bin/compiler ./src/*.cpp
