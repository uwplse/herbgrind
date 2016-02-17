HAMMING=NMSE-example-3.1.out NMSE-example-3.10.out			\
NMSE-example-3.3.out NMSE-example-3.4.out NMSE-example-3.5.out		\
NMSE-example-3.6.out NMSE-example-3.7.out NMSE-example-3.8.out		\
NMSE-example-3.9.out NMSE-p42.out NMSE-problem-3.2.1.out		\
NMSE-problem-3.3.1.out NMSE-problem-3.3.2.out NMSE-problem-3.3.3.out	\
NMSE-problem-3.3.4.out NMSE-problem-3.3.5.out NMSE-problem-3.3.6.out	\
NMSE-problem-3.3.7.out NMSE-problem-3.4.1.out NMSE-problem-3.4.2.out	\
NMSE-problem-3.4.3.out NMSE-problem-3.4.4.out NMSE-problem-3.4.5.out	\
NMSE-problem-3.4.6.out NMSE-section-3.11.out NMSE-section-3.5.out

FLAGS=-g -std=c11 -lm -lmpfr -I../../valgrind/herbgrind/include

%-s.out: %.c single-driver.c
	gcc $(FLAGS) \
		-DNARGS=$(shell grep f_if $*.c | tr '()_ ,' '\n' \
                          | tail -n+2 | grep float -c) \
	-o $@ single-driver.c $<
	chmod u+x $@
HERBIE-PATH=../../../herbie

%.out: %.c driver.c
	gcc $(FLAGS) \
		-DNARGS=$(shell grep f_if $*.c | tr '()_ ,' '\n' \
                          | tail -n+2 | grep float -c) \
        -o $@ driver.c $<
	chmod u+x $@

%_pts.c: %.c

%.c:
	cd $(HERBIE-PATH) && racket herbie/compile/c-single.rkt -p bench/hamming/ ../herbgrind/bench/herbie-crosseval/

all: $(HAMMING)
