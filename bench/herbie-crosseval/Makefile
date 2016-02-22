include ../../Makefile.common

HAMMING=NMSE-example-3.1.out-errors.gh					\
NMSE-example-3.10.out-errors.gh NMSE-example-3.3.out-errors.gh		\
NMSE-example-3.4.out-errors.gh NMSE-example-3.5.out-errors.gh		\
NMSE-example-3.6.out-errors.gh NMSE-example-3.7.out-errors.gh		\
NMSE-example-3.8.out-errors.gh NMSE-example-3.9.out-errors.gh		\
NMSE-p42.out-errors.gh NMSE-problem-3.2.1.out-errors.gh			\
NMSE-problem-3.3.1.out-errors.gh NMSE-problem-3.3.2.out-errors.gh	\
NMSE-problem-3.3.3.out-errors.gh NMSE-problem-3.3.4.out-errors.gh	\
NMSE-problem-3.3.5.out-errors.gh NMSE-problem-3.3.6.out-errors.gh	\
NMSE-problem-3.3.7.out-errors.gh NMSE-problem-3.4.1.out-errors.gh	\
NMSE-problem-3.4.2.out-errors.gh NMSE-problem-3.4.3.out-errors.gh	\
NMSE-problem-3.4.4.out-errors.gh NMSE-problem-3.4.5.out-errors.gh	\
NMSE-problem-3.4.6.out-errors.gh NMSE-section-3.11.out-errors.gh	\
NMSE-section-3.5.out-errors.gh

FLAGS=-g -std=c11 -lm -lmpfr -I../../valgrind/herbgrind/include

RHOST=uwplse.org
RHOSTDIR=/var/www/herbie/herbgrind/hamming

.PHONY: all Makefile ../../Makefile.common

all: $(HAMMING)
	$(eval B=$(git rev-parse --abbrev-ref HEAD))
	$(eval C=$(git rev-parse HEAD | sed 's/\(..........\).*/\1/'))
	$(eval RDIR=$(shell date +%s):$(shell hostname))
	rsync --recursive $(HAMMING) $(RHOST):$(RHOSTDIR)/$(RDIR)

.SECONDARY: %-pts.c %.c
.PRECIOUS: %-pts.c %.c

%-s.out: %.c single-driver.c
	gcc $(FLAGS) \
		-DNARGS=$(shell grep f_if $*.c | tr '()_ ,' '\n' \
                          | tail -n+2 | grep float -c) \
	-o $@ single-driver.c $<
	chmod u+x $@

HERBIE-PATH=../../../herbie
HERBGRIND-PATH=~/herbie-all/herbgrind

%.out: %.c %-pts.c driver.c
	gcc $(FLAGS) \
		-DNARGS=$(shell grep f_if $*.c | tr '()_ ,' '\n' \
                          | tail -n+2 | grep float -c) \
        -o $@ $^
	chmod u+x $@

%.c:
	cd $(HERBIE-PATH) && racket herbie/compile/c-single.rkt --test '$(shell echo $* | sed -e "s/-/ /g" )' \
                                     --prog bench/hamming/ $(HERBGRIND-PATH)/bench/herbie-crosseval/

%-pts.c:
	cd $(HERBIE-PATH) && racket herbie/compile/c-single.rkt --test '$(shell echo $* | sed -e "s/-/ /g" )' \
                                     --pts bench/hamming/ $(HERBGRIND-PATH)/bench/herbie-crosseval/

%.out-errors.gh:
	$(eval PRECISION=$(shell grep mpfr_set_default_prec $*.c | head -n 1 | cut -d '(' -f 2 | cut -d ')' -f 1))
	echo $(PRECISION)
	../../valgrind/$(HG_LOCAL_INSTALL_NAME)/bin/valgrind --tool=herbgrind \
                                         --precision=$(PRECISION) \
                                         --start-off \
                                         ./$*.out
