HerbGrind -- A Valgrind Tool For Herbie
=======================================

While I'm not sure yet whether the best move is to work with FpDebug
to get the results we want, or to write our own valgrind tool, both
paths necessitate learning more about the valgrind internals, and how
FpDebug works. And what better way to do that than writing a valgrind
tool?

For now, this will be more of a testbed for experimentation with
valgrind and the tool interface than an actual usable tool. The first
plan is to get it printing the VEX of a simple program to investigate
the mismatched virtual register issue we've been having with
valgrind. Then, I'll probably try porting some functionality from
FpDebug over to HerbGrind, to get a better sense of what's going on in
FpDebug, and to have some functionality running on a normal platform
(as opposed to a virtual container of an old debian install).


Installation Instructions
-------------------------

Running just "make" or "make compile" after cloning the repo should
just work.

If you just want to configure everything, but not compile, run "make
setup".

If you've modified the source code in the $toplevel/herbgrind
(**NEVER** modify the code in $toplevel/valgrind/herbgrind), and want
to get that code into the valgrind code, run "make update".
