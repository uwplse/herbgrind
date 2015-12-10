all: compile

valgrind/README:
	svn co svn://svn.valgrind.org/valgrind/trunk valgrind

valgrind/herbgrind/Makefile: valgrind/README herbgrind/Makefile.am
	cp -r herbgrind/* valgrind/herbgrind/
	cd valgrind && ./autogen.sh
	cd valgrind && ./configure --prefix=$(shell pwd)/valgrind/inst

setup: valgrind/herbgrind/Makefile

valgrind/inst/lib/valgrind/herbgrind-amd64-linux: herbgrind/hg_main.c
	$(MAKE) setup
	cp -r herbgrind/* valgrind/herbgrind
	$(MAKE) -C valgrind/ install

compile: valgrind/inst/lib/valgrind/herbgrind-amd64-linux
