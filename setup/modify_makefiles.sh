#!/bin/env bash

awk '/^TOOLS =/{print;print "\t\therbgrind \\ ";next}1' ../valgrind/Makefile.am > ../valgrind/Makefile.am
awk '/^AC_CONFIG_FILES([/{print;print "   herbgrind/Makefile";print "   herbgrind/docs/Makefile";print "   herbgrind/tests/Makefile";next}1' ../valgrind/configure.ac > ../valgrind/Makefile.am
