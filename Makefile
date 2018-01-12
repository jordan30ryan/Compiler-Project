# Super basic makefile

CC=g++
CFLAGS=-Wall --std=c++11 -ggdb

test: ./src/*.cpp
	$(CC) $(CFLAGS) -o ./bin/test ./src/*.cpp
