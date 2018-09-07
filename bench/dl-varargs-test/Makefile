
FLAGS=-g -std=c11 -Wall -Werror -I../../valgrind/herbgrind/include/

CC ?= gcc

all: libdl.so main.out

libdl.so: dl.c dl.h
	$(CC) $(FLAGS) -fpic -c dl.c
	$(CC) -shared -o libdl.so dl.o

main.out: main.c dl.h
	$(CC) -L$(CURDIR) -Wl,-rpath=$(CURDIR) $(FLAGS) -o main.out main.c -ldl
