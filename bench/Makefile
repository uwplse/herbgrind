
FLAGS=-g -std=c11 -lm -lmpfr -I../valgrind/herbgrind/include/

CSRC = $(wildcard *.c)
COUT = $(patsubst %.c,%.out,$(CSRC))

# NOTE : does not build mini-nocall.sout (trouble on osx)
all: $(COUT)

%.out: %.c
	gcc $(FLAGS) -o $@ $<
	chmod u+x $@

%.sout: %.s
	gcc $(FLAGS) -o $@ $<
	chmod u+x $@

%.s: %.c
	gcc $(FLAGS) -S -o $@ $<

clean:
	rm -rf *.out *.sout *.dSYM
