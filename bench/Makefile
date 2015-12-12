
FLAGS=-g -std=c11 -lm -lmpfr

all: testprog.out mini.out miniasm.sout tiny.out

%.out: %.c
	gcc $(FLAGS) -o $@ $<
	chmod u+x $@

%.sout: %.s
	gcc $(FLAGS) -o $@ $<
	chmod u+x $@

clean:
	rm *.out *.sout
