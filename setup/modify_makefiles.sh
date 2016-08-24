#!/usr/bin/env bash

#
#--------------------------------------------------------------------*/
#--- HerbGrind: a valgrind tool for Herbie              hg_main.c ---*/
#--------------------------------------------------------------------*/
#
#
# This file is part of HerbGrind, a valgrind tool for diagnosing
# floating point accuracy problems in binary programs and extracting
# problematic expressions.
#
# Copyright (C) 2016 Alex Sanchez-Stern
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307, USA.
#
# The GNU General Public License is contained in the file COPYING.
#

# Modify the valgrind makefiles to include the herbgrind directory
gawk '/^TOOLS =/{print;print "\t\therbgrind \\";next}1' ../valgrind/Makefile.am > ../valgrind/Makefile.am.tmp
mv ../valgrind/Makefile.am.tmp ../valgrind/Makefile.am
gawk '/^AC_CONFIG_FILES/ && !seen {print;print "   herbgrind/Makefile";print "   herbgrind/docs/Makefile";print "   herbgrind/tests/Makefile";seen=1;next}1' ../valgrind/configure.ac > ../valgrind/configure.ac.tmp
mv ../valgrind/configure.ac.tmp ../valgrind/configure.ac
gawk '/^TOOL_DEPENDENCIES_@VGCONF_PLATFORM_PRI_CAPS@ =/{print;print "\t$(extra_deps_amd64) \\";next}1' ../valgrind/Makefile.tool.am > ../valgrind/Makefile.tool.am.tmp
mv ../valgrind/Makefile.tool.am.tmp ../valgrind/Makefile.tool.am.tmp
gawk '/^TOOL_DEPENDENCIES_@VGCONF_PLATFORM_SEC_CAPS@ =/{print;print "\t$(extra_deps_i386) \\";next}1' ../valgrind/Makefile.tool.am > ../valgrind/Makefile.tool.am.tmp
mv ../valgrind/Makefile.tool.am.tmp ../valgrind/Makefile.tool.am

# Comment out uses of fprintf in assert.c in gmp, since running on top
# of the valgrind c library it doesn't work. Not sure if we actually
# need this yet.

# sed -i -e '/fprintf/s/^/\/\//' -e '/if (linenum != -1)/s/^/\/\//' ../gmp/assert.c
