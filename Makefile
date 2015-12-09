all: compile

valgrind/README:
	svn co svn://svn.valgrind.org/valgrind/trunk valgrind

valgrind/configure:
	./valgrind/autogen.sh

valgrind/Makefile: valgrind/configure
	./valgrind/configure --prefix=$(shell pwd)/valgrind/inst

valgrind/herbgrind/:
	cp -r herbgrind valgrind/herbgrind 

setup: valgrind/README valgrind/herbgrind/ valgrind/Makefile

update:
	cp -r herbgrind valgrind/herbgrind

compile: setup update
	$(MAKE) -C valgrind/ install
