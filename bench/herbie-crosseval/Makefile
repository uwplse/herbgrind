FLAGS=-g -std=c11 -lm -lmpfr -I../../valgrind/herbgrind/include

%.out: %.c driver.c
	gcc $(FLAGS) -DNARGS=$(shell grep f_if $*.c | tr '()_ ,' '\n' | tail -n+2 | grep float -c) \
                     -o $@ driver.c $<
	chmod u+x $@

