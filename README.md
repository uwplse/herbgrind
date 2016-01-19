HerbGrind -- A Valgrind Tool For Herbie
=======================================

<img src="logo.jpg" alt="Herbgrind logo" width="150"/>

<!-- While I'm not sure yet whether the best move is to work with FpDebug -->
<!-- to get the results we want, or to write our own valgrind tool, both -->
<!-- paths necessitate learning more about the valgrind internals, and how -->
<!-- FpDebug works. And what better way to do that than writing a valgrind -->
<!-- tool? -->

<!-- For now, this will be more of a testbed for experimentation with -->
<!-- valgrind and the tool interface than an actual usable tool. The first -->
<!-- plan is to get it printing the VEX of a simple program to investigate -->
<!-- the mismatched virtual register issue we've been having with -->
<!-- valgrind. Then, I'll probably try porting some functionality from -->
<!-- FpDebug over to HerbGrind, to get a better sense of what's going on in -->
<!-- FpDebug, and to have some functionality running on a normal platform -->
<!-- (as opposed to a virtual container of an old debian install). -->

Herbgrind is a valgrind tool for tracking floating point inprecision
in binary programs, inspired by FpDebug. Herbgrind is still pretty
early in development, but it's goals are:

- Determining whether a program is generally accurate

- Identifying places in source code of a binary program where
  inaccuracy occurs

- Identifying input distributions of unstable numerical code

- Extracting enough other information from programs to make
  improvement with herbie easy

Installation Instructions
-------------------------

The project was developed on a 64-bit linux platform that supports
32-bit compilation. If you're running on a platform like this, running
just "make" or "make compile" after cloning the repo should just work.
Otherwise, you'll want to open up the toplevel makefile, and modify
some variables. Set TARGET\_PLAT and ARCH\_PRI appropriately for your
platform. If you're on a platform for which valgrind wants to build a
secondary version (certain 64-bit configurations will cause valgrind
to want to do 32-bit too), then set ARCH\_SEC to that secondary
architecture.

If you just want to configure everything, but not compile, run "make
setup".

**NEVER** modify the code in $toplevel/valgrind/herbgrind, only modify
$toplevel/herbgrind. $toplevel/valgrind/herbgrind gets overwritten on
every build.

Running
-------

To run, just run "*path-to-herbgrind*/valgrind/inst/bin/valgrind --tool=herbgrind *executable-to-run-on*"
