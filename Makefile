# Super basic makefile

CC=g++
CFLAGS=-Wall

test: ./src/*.cpp
	$(CC) -o ./bin/test ./src/*.cpp
