include Makefile.common

SHELL=/bin/bash

# The versions of gmp and mpfr, for matching on the archive names.
GMP_VERSION=6.1.0
MPFR_VERSION=3.1.3
OPENLIBM_VERSION=0.4.1
# The repo to clone valgrind from.
VALGRIND_REPO_LOCATION=svn://svn.valgrind.org/valgrind/trunk
VALGRIND_REVISION=15800
VEX_REPO_LOCATION=svn://svn.valgrind.org/vex/trunk
VEX_REVISION=3210
# The architecture thhat we're buiding herbgrind for, in the syntax of
# valgrind filename conventions for this sort of thing.
TARGET_PLAT:=$(shell test `uname` = "Darwin" && echo "amd64-darwin" || echo "amd64-linux")
ARCH_PRI=amd64
ARCH_SEC=

ifdef ARCH_SEC
DEPS = deps/gmp-64/README deps/mpfr-64/README deps/gmp-32/README deps/mpfr-32/README
else
DEPS = deps/gmp-64/herbgrind-install/lib/libgmp.a	\
deps/mpfr-64/herbgrind-install/lib/libmpfr.a		\
deps/openlibm-64/libopenlibm.a
endif

HEADERS=src/include/herbgrind.h src/helper/mpfr-valgrind-glue.h		\
src/helper/stack.h src/helper/instrument-util.h				\
src/helper/runtime-util.h src/helper/ir-info.h src/helper/debug.h	\
src/helper/list.h src/helper/xarray.h src/helper/bbuf.h src/options.h	\
src/runtime/value-shadowstate/shadowval.h				\
src/runtime/value-shadowstate/value-shadowstate.h			\
src/runtime/value-shadowstate/exprs.h					\
src/runtime/value-shadowstate/exprs.hh					\
src/runtime/value-shadowstate/real.h					\
src/runtime/value-shadowstate/pos-tree.h				\
src/runtime/value-shadowstate/range.h					\
src/runtime/op-shadowstate/shadowop-info.h				\
src/runtime/op-shadowstate/marks.h					\
src/runtime/op-shadowstate/output.h src/runtime/shadowop/shadowop.h	\
src/runtime/shadowop/conversions.h src/runtime/shadowop/realop.h	\
src/runtime/shadowop/error.h src/runtime/shadowop/mathreplace.h		\
src/runtime/shadowop/symbolic-op.h					\
src/runtime/shadowop/influence-op.h src/runtime/shadowop/local-op.h	\
src/runtime/shadowop/exit-float-op.h src/instrument/instrument.h	\
src/instrument/instrument-op.h src/instrument/instrument-storage.h	\
src/instrument/conversion.h src/instrument/semantic-op.h		\
src/instrument/ownership.h src/instrument/floattypes.h

SOURCES=src/hg_main.c src/helper/mathwrap.c src/helper/printf-wrap.c	\
src/include/mk-mathreplace.py src/helper/mpfr-valgrind-glue.c		\
src/helper/stack.c src/helper/instrument-util.c				\
src/helper/runtime-util.c src/helper/ir-info.c src/helper/bbuf.c	\
src/options.c src/runtime/value-shadowstate/shadowval.c			\
src/runtime/value-shadowstate/value-shadowstate.c			\
src/runtime/value-shadowstate/shadowval.c				\
src/runtime/value-shadowstate/exprs.c					\
src/runtime/value-shadowstate/real.c					\
src/runtime/value-shadowstate/pos-tree.c				\
src/runtime/value-shadowstate/range.c					\
src/runtime/op-shadowstate/shadowop-info.c				\
src/runtime/op-shadowstate/marks.c					\
src/runtime/op-shadowstate/output.c src/runtime/shadowop/shadowop.c	\
src/runtime/shadowop/conversions.c src/runtime/shadowop/realop.c	\
src/runtime/shadowop/error.c src/runtime/shadowop/mathreplace.c		\
src/runtime/shadowop/symbolic-op.c					\
src/runtime/shadowop/influence-op.c src/runtime/shadowop/local-op.c	\
src/runtime/shadowop/exit-float-op.c src/instrument/instrument.c	\
src/instrument/instrument-op.c src/instrument/instrument-storage.c	\
src/instrument/conversion.c src/instrument/semantic-op.c		\
src/instrument/ownership.c src/instrument/floattypes.c

all: compile

# We use the README file of a repo as a proxy for whether or not that
# repo currently exists.
valgrind/README:
# Check out valgrind from source.
	svn co -q --ignore-externals $(VALGRIND_REPO_LOCATION)@$(VALGRIND_REVISION) valgrind
	svn co -q $(VEX_REPO_LOCATION)@$(VEX_REVISION) valgrind/VEX
# Make a directory for the herbgrind tool
	mkdir valgrind/herbgrind
# ...and copy the files from the top level herbgrind folder into it.
	cp -r src/* valgrind/herbgrind/

# The herbgrind makefile needs to be recreated, if it's source .am
# file changes or we've just cloned the valgrind repo
valgrind/herbgrind/Makefile: valgrind/Makefile src/Makefile.am
# Copy over the latest version of all the herbgrind stuff, including
# the .am file that we need for this step.
	rm -r -f valgrind/herbgrind/*
	mkdir -p valgrind/herbgrind
	cp -r src/* valgrind/herbgrind/
	cd valgrind && ./autogen.sh
	cd valgrind && \
		CFLAGS="-fno-stack-protector" \
		./configure --prefix=$(shell pwd)/valgrind/$(HG_LOCAL_INSTALL_NAME) \
		            --enable-only64bit \
		            --build=$(TARGET_PLAT)

valgrind/Makefile: valgrind/README
# Run a script to modify the setup files to include the herbgrind
# directory.
	svn revert --depth=infinity valgrind
	cd setup && ./modify_makefiles.sh
# Run the autogen and configure scripts to turn the .am file into a
# real makefile.
	cd valgrind && ./autogen.sh
	cd valgrind && \
		CFLAGS="-fno-stack-protector" \
		./configure --prefix=$(shell pwd)/valgrind/$(HG_LOCAL_INSTALL_NAME) \
		            --enable-only64bit \
		            --build=$(TARGET_PLAT)

# This is the target we call to bring in the dependencies, like gmp,
# mpfr, and valgrind, and to make sure the herbgrind files have been
# initially copied over.
setup: valgrind/Makefile $(DEPS)

# This is the target we call to actually get the executable built so
# we can run herbgrind.
valgrind/$(HG_LOCAL_INSTALL_NAME)/lib/valgrind/herbgrind-$(TARGET_PLAT): $(SOURCES) $(HEADERS) src/Makefile.am setup
# Then, let's run the python script to generate the mathreplace header
# in src/
	rm -rf src/include/mathreplace-funcs.h
	cd src/include/ && python mk-mathreplace.py
# Copy over the herbgrind sources again, because why the hell not.
	cp -r src/* valgrind/herbgrind
# Run make install to build the binaries and put them in the right
# place.
ifeq ($(DONT_WRAP),)
	$(MAKE) -C valgrind/ install
else
	$(MAKE) -C valgrind CFLAGS+=-DDONT_WRAP install
endif

# Alias the compile target to just "compile" for ease of use
compile: valgrind/$(HG_LOCAL_INSTALL_NAME)/lib/valgrind/herbgrind-$(TARGET_PLAT)

# Use the gmp README to tell if gmp has been extracted yet.
deps/gmp-%/herbgrind-install/lib/libgmp.a: setup/gmp-$(GMP_VERSION).tar.xz setup/patch_gmp.sh
# Extract gmp, and rename its folder so we don't have to use the
# version number all over the place.
	tar xf setup/gmp-$(GMP_VERSION).tar.xz
	mkdir -p deps
	mv gmp-$(GMP_VERSION) deps/gmp-$*
# Touch the README to update its timestamp so that we don't build it
# again next time unless the archive changes.
	touch deps/gmp-$*/README
# Patch the gmp files to remove instances of memory functions which
# will fail to link with the valgrind partial c library.
	cd setup && ./patch_gmp.sh $*
# Configure and make it, putting its output in the install folder
# locally instead of in a global location, so it doesn't conflict with
# other versions of gmp.
	cd deps/gmp-$*/ && \
		CFLAGS="-fno-stack-protector" \
		ABI=$* \
		./configure --prefix=$(shell pwd)/deps/gmp-$*/$(HG_LOCAL_INSTALL_NAME)
	$(MAKE) -C deps/gmp-$*
	$(MAKE) -C deps/gmp-$* install

deps/openlibm-%/libopenlibm.a: setup/openlibm-$(OPENLIBM_VERSION).tar.gz
	tar xf setup/openlibm-$(OPENLIBM_VERSION).tar.gz
	mkdir -p deps
	mv openlibm-$(OPENLIBM_VERSION) deps/openlibm-$*
	touch deps/openlibm-$*/README.md
	CFLAGS+="-fno-stack-protector" \
	$(MAKE) -C deps/openlibm-$*

# Adding this flag ensures that MPFR doesn't allocate any of it's
# variables as thread local. This is important because valgrind moves
# around all the tool memory, so we get lots of problems if we let the
# linker try to put thread local variables on our stack. Luckily,
# herbgrind doesn't need threads, nor does valgrind, and client
# programs will be serialized by valgrind, so it's safe to disable
# these.
MPFR_CONFIGURE_FLAGS = --disable-thread-safe

configure-mpfr-32:
	cd deps/mpfr-32/ && \
		CFLAGS="-fno-stack-protector" \
		./configure --prefix=$(shell pwd)/deps/mpfr-32/$(HG_LOCAL_INSTALL_NAME) \
		            --with-gmp-build=$(shell pwd)/deps/gmp-32 \
		            --build=i386 \
		            $(MPFR_CONFIGURE_FLAGS) &&
		aclocal

configure-mpfr-64:
	cd deps/mpfr-64/ && \
		CFLAGS="-fno-stack-protector" \
		./configure --prefix=$(shell pwd)/deps/mpfr-64/$(HG_LOCAL_INSTALL_NAME) \
		            --with-gmp-build=$(shell pwd)/deps/gmp-64 \
		            --build=amd64 \
		            $(MPFR_CONFIGURE_FLAGS) && \
		aclocal

# Use the mpfr readme to tell if mpfr has been extracted yet.
deps/mpfr-%/herbgrind-install/lib/libmpfr.a: setup/mpfr-$(MPFR_VERSION).tar.xz setup/patch_mpfr.sh
# Extract mpfr, and rename its folder so we don't have to use the
# version number all over the place.
	tar xf setup/mpfr-$(MPFR_VERSION).tar.xz
	mkdir -p deps
	rm -rf deps/mpfr-$*
	mv mpfr-$(MPFR_VERSION) deps/mpfr-$*
# Touch the README to update its timestamp so that we don't build it
# again next time unless the archive changes.
	touch deps/mpfr-$*/README
# Patch the mpfr files to allow us to use alternative memory functions
# which will not fail at link time.
	cd setup && ./patch_mpfr.sh $*
# Configure and make mpfr. We want to use the gmp we built locally for
# this, and we'll install it locally too for the same reasons as
# above.
	$(MAKE) configure-mpfr-$*
	$(MAKE) -C deps/mpfr-$*
	$(MAKE) -C deps/mpfr-$* install

wc:
	wc $(SOURCES) $(HEADERS)
loc:
	sloccount $(SOURCES) $(HEADERS)

clean:
	touch valgrind/README

clean-deps:
	rm -rf valgrind/ deps/

clean-files:
	rm vgcore*

clear-preload:
	rm valgrind/$(HG_LOCAL_INSTALL_NAME)/lib/vgpreload_herbgrind*

.PHONY: test backup-logs

TESTS=$(wildcard bench/*.out-errors.gh.expected)

bench/%.out: bench/%.c
	$(MAKE) -C bench $*.out

# The .out version is the binary; TESTS stores the expected output files
test: compile $(TESTS) $(TESTS:.out-errors.gh.expected=.out)
	python3 bench/test.py $(TESTS:.out-errors.gh.expected=.out)

backup-logs:
	tar czf logs.tar.gz logs
	rsync logs.tar.gz uwplse.org:/var/www/herbie/herbgrind/$(shell hostname)_logs.tar.gz
	rm logs.tar.gz
