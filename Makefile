# Super basic makefile

CC=g++
CFLAGS=-Wall --std=c++11

test: ./src/*.cpp
	$(CC) -o ./bin/test ./src/*.cpp
