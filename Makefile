# Super basic makefile

CC=g++
CFLAGS=-Wall --std=c++11 -ggdb -Wno-char-subscripts

test: ./src/*.cpp
	$(CC) $(CFLAGS) -o ./bin/test ./src/*.cpp
