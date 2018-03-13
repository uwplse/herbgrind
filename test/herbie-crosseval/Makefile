include ../../Makefile.common

HAMMING=NMSE-example-3.1.out.gh					\
NMSE-example-3.10.out.gh NMSE-example-3.3.out.gh		\
NMSE-example-3.4.out.gh NMSE-example-3.5.out.gh		\
NMSE-example-3.6.out.gh NMSE-example-3.7.out.gh		\
NMSE-example-3.8.out.gh NMSE-example-3.9.out.gh		\
NMSE-p42,-positive.out.gh NMSE-p42,-negative.out.gh	\
NMSE-problem-3.2.1,-positive.out.gh				\
NMSE-problem-3.2.1,-negative.out.gh				\
NMSE-problem-3.3.1.out.gh NMSE-problem-3.3.2.out.gh	\
NMSE-problem-3.3.3.out.gh NMSE-problem-3.3.4.out.gh	\
NMSE-problem-3.3.5.out.gh NMSE-problem-3.3.6.out.gh	\
NMSE-problem-3.3.7.out.gh NMSE-problem-3.4.1.out.gh	\
NMSE-problem-3.4.2.out.gh NMSE-problem-3.4.3.out.gh	\
NMSE-problem-3.4.4.out.gh NMSE-problem-3.4.5.out.gh	\
NMSE-problem-3.4.6.out.gh NMSE-section-3.11.out.gh	\
NMSE-section-3.5.out.gh

FLAGS=-g -std=c11 -lm -I../../valgrind/herbgrind/include

RHOST=uwplse.org
RHOSTDIR=/var/www/herbie/herbgrind/hamming

NPTS=8000
SEED=

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
		-DNARGS=$(shell grep f $*.c | tr '()_ ,' '\n' \
                          | tail -n+2 | grep double -c) \
		-DNPTS=$(shell grep "double pts" $*-pts.c | \
			cut -d '[' -f 2 | cut -d ']' -f 1) \
        -o $@ $^
	chmod u+x $@

driver.c:
	true

%.c:
	racket fpcore-to-c.rkt $(HERBIE-PATH)/bench/hamming/ \
		'$(shell echo $* | sed -e "s/-/ /g" )' \
		> $@

%-pts.c:
ifeq ($(SEED),)
	racket sample-fpcore-pts.rkt \
		-n $(NPTS) \
		$(HERBIE-PATH)/bench/hamming \
		'$(shell echo $* | sed -e "s/-/ /g" )' \
		> $@
else
	racket sample-fpcore-pts.rkt \
		-n $(NPTS) \
		-r "$(SEED)" \
		$(HERBIE-PATH)/bench/hamming \
		'$(shell echo $* | sed -e "s/-/ /g" )' \
		> $@
endif

%.out.gh: %.out %-pts.c
ifeq ($(PRECISION),)
	$(eval PRECISION=$(shell grep "#define PRECISION" $*-pts.c \
	| head -n 1 | cut -d ' ' -f 3 ))
endif
	../../valgrind/$(HG_LOCAL_INSTALL_NAME)/bin/valgrind \
					--tool=herbgrind \
                                        --precision=$(PRECISION) \
					--error-threshold=0 \
                                        --start-off \
                                        ./$*.out
