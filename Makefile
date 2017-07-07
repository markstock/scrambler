CC=g++
CFLAGS=-ansi -pedantic -Wall -Wextra -O3

all : scrambler

scrambler : lodepng.cpp scrambler.cpp
	$(CC) $(CFLAGS) $^ -o $@


