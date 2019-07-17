Herbgrind -- A Valgrind Tool For Herbie
=======================================

<img src="docs/logo.jpg" alt="Herbgrind logo" width="50%"/>

Herbgrind is a valgrind tool for tracking floating point imprecision
in binary programs, inspired by FpDebug. Herbgrind is still pretty
early in development, but its goals are:

- Determining whether a program is generally accurate

- Identifying places in source code of a binary program where
  inaccuracy occurs

- Identifying input distributions of unstable numerical code

- Extracting enough other information from programs to make
  improvement with herbie easy

Herbgrind is documented at http://uwplse.github.io/herbgrind/

Herbgrind is licensed under GPLv3 (and some dependencies are under
other, similar licenses). If that doesn't work for your use case, let
us know, and we're happy to talk about licensing under other terms.

Installation Instructions
-------------------------

### Prerequisites

You'll need `git`, `autotools`, and `python3` installed to build herbgrind. You
can most likely find them in your linux distributions package repositories,
or Homebrew if you're on OS X.

### Installing

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

To build on OS X, you need XCode Command Line Tools, which you can install
with:

    $ xcode-select --install

You will also need the GNU versions of `awk` and `sed`.  The best way to
install these is through [Homebrew](http://brew.sh/):

    $ brew install gawk
    $ brew install gnu-sed --with-default-names

Since January 2019, you'll need to follow these instructions to get default names:

https://stackoverflow.com/questions/30003570/how-to-use-gnu-sed-on-mac-os-x/34815955#34815955

Running
-------

To run, just run "*path-to-herbgrind*/valgrind/herbgrind-install/bin/valgrind --tool=herbgrind *executable-to-run-on*"

<img src="docs/logo-drawing.png" alt="Herbgrind logo" width="50%"/>
