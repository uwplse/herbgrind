GMP_VERSION=6.1.0
VALGRIND_REPO_LOCATION=svn://svn.valgrind.org/valgrind/trunk
TARGET_ARCH=amd64-linux

all: compile

valgrind/README:
	svn co $(VALGRIND_REPO_LOCATION) valgrind
	cd setup && ./modify_makefiles.sh
	mkdir valgrind/herbgrind
	cp -r herbgrind/* valgrind/herbgrind/

valgrind/herbgrind/Makefile: valgrind/README herbgrind/Makefile.am
	cp -r herbgrind/* valgrind/herbgrind/
	cd valgrind && ./autogen.sh
	cd valgrind && ./configure --prefix=$(shell pwd)/valgrind/inst

setup: valgrind/herbgrind/Makefile gmp/README

valgrind/inst/lib/valgrind/herbgrind-$(TARGET_ARCH): herbgrind/hg_main.c
	$(MAKE) setup
	cp -r herbgrind/* valgrind/herbgrind
	$(MAKE) -C valgrind/ install

compile: valgrind/inst/lib/valgrind/herbgrind-$(TARGET_ARCH)

gmp/README: setup/gmp-$(GMP_VERSION).tar.xz
	tar xf setup/gmp-$(GMP_VERSION).tar.xz
	mv gmp-$(GMP_VERSION) gmp
	touch gmp/README
	cd gmp/ && ./configure --prefix=$(shell pwd)gmp/install
	$(MAKE) -C gmp
	$(MAKE) -C gmp install
