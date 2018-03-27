FLAGS=-g -std=c11 -lm

CSRC = $(wildcard *.c)
COUT = $(patsubst %.c,%.out,$(CSRC))

CC ?= gcc

# NOTE : does not build mini-nocall.sout (trouble on osx)
all: $(COUT)

testprog.out: testprog.c
	$(CC) -o $@ $< $(FLAGS) -lmpfr
	chmod u+x $@

%.out: %.c
	$(CC) -o $@ $< $(FLAGS)
	chmod u+x $@

%.sout: %.s
	$(CC) -o $@ $< $(FLAGS)
	chmod u+x $@

%.s: %.c
	$(CC) $(FLAGS) -S -o $@ $<

clean:
	rm -rf *.out *.sout *.dSYM
