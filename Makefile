all:

valgrind/README:
	svn co svn://svn.valgrind.org/valgrind/trunk valgrind

valgrind/herbgrind/:
	ln -s ../herbgrind valgrind/herbgrind 

setup: valgrind/README valgrind/herbgrind/
