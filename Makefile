SHELL := /bin/sh

CC ?= cc
LD := $(CC)

CFLAGS := -std=c99 -O0 -g -Wall -Wextra -Werror -Wno-switch -Wno-unused-const-variable
LDFLAGS := -lm -fsanitize=address,leak,undefined

all: expr

expr: string.o list.o lexer.o parser.o transform.o main.o
	$(LD) -o $@ $(LDFLAGS) $^

.c.o:
	$(CC) -o $@ $(CFLAGS) -c $^

clean:
	rm expr
	rm *.o

.PHONY: all clean
