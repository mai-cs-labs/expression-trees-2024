SHELL := /bin/sh

CC ?= cc
LD := $(CC)

CFLAGS := -std=c99 -O0 -g -Wall -Wextra -Werror
LDFLAGS := -fsanitize=address,leak,undefined

all: expr

expr: string.o list.o tree.o lexer.o parser.o main.o
	$(LD) -o $@ $(LDFLAGS) $^

.c.o:
	$(CC) -o $@ $(CFLAGS) -c $^

clean:
	rm expr
	rm *.o

.PHONY: all clean
