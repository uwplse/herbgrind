
FLAGS=-g -std=c11 -lm -lmpfr -I../valgrind/herbgrind

all: testprog.out mini.out tiny.out mini-nocall.sout

%.out: %.c
	gcc $(FLAGS) -o $@ $<
	chmod u+x $@

%.sout: %.s
	gcc $(FLAGS) -o $@ $<
	chmod u+x $@

%.s: %.c
	gcc $(FLAGS) -S -o $@ $<

clean:
	rm *.out *.sout
