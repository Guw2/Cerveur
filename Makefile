CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c11 -g -Iinclude
SRC=$(wildcard src/*.c)
OUT=cerveur

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run: all
	./$(OUT)

clean:
	rm -f $(OUT)
