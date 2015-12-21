#!/bin/env bash

# Modify the valgrind makefiles to include the herbgrind directory
awk -i inplace '/^TOOLS =/{print;print "\t\therbgrind \\";next}1' ../valgrind/Makefile.am
awk -i inplace '/^AC_CONFIG_FILES/ && !seen {print;print "   herbgrind/Makefile";print "   herbgrind/docs/Makefile";print "   herbgrind/tests/Makefile";seen=1;next}1' ../valgrind/configure.ac
awk -i inplace '/^TOOL_DEPENDENCIES_@VGCONF_PLATFORM_PRI_CAPS@ =/{print;print "\t$(extra_deps) \";next}1' ../valgrind/Makefile.tool.am

# Comment out uses of fprintf in assert.c in gmp, since running on top
# of the valgrind c library it doesn't work. Not sure if we actually
# need this yet.

# sed -i -e '/fprintf/s/^/\/\//' -e '/if (linenum != -1)/s/^/\/\//' ../gmp/assert.c
