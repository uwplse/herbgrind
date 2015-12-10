#!/bin/env bash

awk -i inplace '/^TOOLS =/{print;print "\t\therbgrind \\";next}1' ../valgrind/Makefile.am
awk -i inplace '/^AC_CONFIG_FILES/ && !seen {print;print "   herbgrind/Makefile";print "   herbgrind/docs/Makefile";print "   herbgrind/tests/Makefile";seen=1;next}1' ../valgrind/configure.ac
