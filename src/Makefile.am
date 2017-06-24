include ../../Makefile.common

extra_deps_amd64 = \
	$(top_builddir)/../deps/mpfr-64/$(HG_LOCAL_INSTALL_NAME)/lib/libmpfr.a \
	$(top_builddir)/../deps/gmp-64/$(HG_LOCAL_INSTALL_NAME)/lib/libgmp.a \
	$(top_builddir)/../deps/openlibm-64/libopenlibm.a

extra_deps_i386 = \
	$(top_builddir)/../deps/mpfr-32/$(HG_LOCAL_INSTALL_NAME)/lib/libmpfr.a \
	$(top_builddir)/../deps/gmp-32/$(HG_LOCAL_INSTALL_NAME)/lib/libgmp.a

include $(top_srcdir)/Makefile.tool.am

EXTRA_DIST = docs/hg-manual.xml

#----------------------------------------------------------------------------
# herbgrind-<platform>
#----------------------------------------------------------------------------

noinst_PROGRAMS  = herbgrind-@VGCONF_ARCH_PRI@-@VGCONF_OS@
if VGCONF_HAVE_PLATFORM_SEC
noinst_PROGRAMS += herbgrind-@VGCONF_ARCH_SEC@-@VGCONF_OS@
endif

HERBGRIND_SOURCES_COMMON = hg_main.c helper/mpfr-valgrind-glue.c	\
helper/stack.c helper/instrument-util.c helper/runtime-util.c		\
helper/ir-info.c helper/bbuf.c						\
runtime/value-shadowstate/value-shadowstate.c				\
runtime/value-shadowstate/shadowval.c					\
runtime/value-shadowstate/exprs.c runtime/value-shadowstate/real.c	\
runtime/value-shadowstate/pos-tree.c					\
runtime/op-shadowstate/shadowop-info.c runtime/op-shadowstate/marks.c	\
runtime/op-shadowstate/output.c runtime/shadowop/shadowop.c		\
runtime/shadowop/realop.c runtime/shadowop/conversions.c		\
runtime/shadowop/error.c runtime/shadowop/symbolic-op.c			\
runtime/shadowop/influence-op.c runtime/shadowop/mathreplace.c		\
runtime/shadowop/local-op.c runtime/shadowop/exit-float-op.c		\
options.c instrument/instrument.c instrument/instrument-op.c		\
instrument/instrument-storage.c instrument/conversion.c			\
instrument/semantic-op.c instrument/floattypes.c			\
instrument/ownership.c

herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_SOURCES      = \
	$(HERBGRIND_SOURCES_COMMON)
herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_CPPFLAGS     = \
	$(AM_CPPFLAGS_@VGCONF_PLATFORM_PRI_CAPS@)
herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_CFLAGS       = \
	$(AM_CFLAGS_@VGCONF_PLATFORM_PRI_CAPS@) \
	-g -O2 -Werror -Wall \
	-I$(top_srcdir)/../deps/gmp-64/$(HG_LOCAL_INSTALL_NAME)/include/ \
	-I$(top_srcdir)/../deps/mpfr-64/$(HG_LOCAL_INSTALL_NAME)/include/
herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_DEPENDENCIES = \
	$(TOOL_DEPENDENCIES_@VGCONF_PLATFORM_PRI_CAPS@) \
	$(extra_deps_amd64)
herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_LDADD        = \
	$(TOOL_LDADD_@VGCONF_PLATFORM_PRI_CAPS@) \
	$(extra_deps_amd64)
herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_LDFLAGS      = \
	$(TOOL_LDFLAGS_@VGCONF_PLATFORM_PRI_CAPS@)
herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_LINK = \
	$(top_builddir)/coregrind/link_tool_exe_@VGCONF_OS@ \
	@VALT_LOAD_ADDRESS_PRI@ \
	$(LINK) \
	$(herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_CFLAGS) \
	$(herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_LDFLAGS)

if VGCONF_HAVE_PLATFORM_SEC
herbgrind_@VGCONF_ARCH_SEC@_@VGCONF_OS@_SOURCES      = \
	$(HERBGRIND_SOURCES_COMMON)
herbgrind_@VGCONF_ARCH_SEC@_@VGCONF_OS@_CPPFLAGS     = \
	$(AM_CPPFLAGS_@VGCONF_PLATFORM_SEC_CAPS@)
herbgrind_@VGCONF_ARCH_SEC@_@VGCONF_OS@_CFLAGS       = \
	$(AM_CFLAGS_@VGCONF_PLATFORM_SEC_CAPS@) \
	-g -Werror -Wall\
	-I$(top_srcdir)/../deps/gmp-32/$(HG_LOCAL_INSTALL_NAME)/include/ \
	-I$(top_srcdir)/../deps/mpfr-32/$(HG_LOCAL_INSTALL_NAME)/include/
herbgrind_@VGCONF_ARCH_SEC@_@VGCONF_OS@_DEPENDENCIES = \
	$(TOOL_DEPENDENCIES_@VGCONF_PLATFORM_SEC_CAPS@)
herbgrind_@VGCONF_ARCH_SEC@_@VGCONF_OS@_LDADD        = \
	$(TOOL_LDADD_@VGCONF_PLATFORM_SEC_CAPS@)
herbgrind_@VGCONF_ARCH_SEC@_@VGCONF_OS@_LDFLAGS      = \
	$(TOOL_LDFLAGS_@VGCONF_PLATFORM_SEC_CAPS@)
herbgrind_@VGCONF_ARCH_SEC@_@VGCONF_OS@_LINK = \
	$(top_builddir)/coregrind/link_tool_exe_@VGCONF_OS@ \
	@VALT_LOAD_ADDRESS_SEC@ \
	$(LINK) \
	$(herbgrind_@VGCONF_ARCH_SEC@_@VGCONF_OS@_CFLAGS) \
	$(herbgrind_@VGCONF_ARCH_SEC@_@VGCONF_OS@_LDFLAGS)
endif

#----------------------------------------------------------------------------
# vgpreload_herbgrind-<platform>.so
#----------------------------------------------------------------------------

noinst_PROGRAMS += vgpreload_herbgrind-@VGCONF_ARCH_PRI@-@VGCONF_OS@.so
if VGCONF_HAVE_PLATFORM_SEC
noinst_PROGRAMS += vgpreload_herbgrind-@VGCONF_ARCH_SEC@-@VGCONF_OS@.so
endif

VGPRELOAD_HERBGRIND_SOURCES_COMMON = helper/mathwrap.c helper/printf-wrap.c

vgpreload_herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_so_SOURCES      = \
	$(VGPRELOAD_HERBGRIND_SOURCES_COMMON)
vgpreload_herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_so_CPPFLAGS     = \
	$(AM_CPPFLAGS_@VGCONF_PLATFORM_PRI_CAPS@)
vgpreload_herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_so_CFLAGS       = \
	$(AM_CFLAGS_PSO_@VGCONF_PLATFORM_PRI_CAPS@)
vgpreload_herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_so_DEPENDENCIES =
vgpreload_herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_so_LDFLAGS      = \
	$(PRELOAD_LDFLAGS_@VGCONF_PLATFORM_PRI_CAPS@)

if VGCONF_HAVE_PLATFORM_SEC
vgpreload_herbgrind_@VGCONF_ARCH_SEC@_@VGCONF_OS@_so_SOURCES      = \
	$(VGPRELOAD_HERBGRIND_SOURCES_COMMON)
vgpreload_herbgrind_@VGCONF_ARCH_SEC@_@VGCONF_OS@_so_CPPFLAGS     = \
	$(AM_CPPFLAGS_@VGCONF_PLATFORM_SEC_CAPS@)
vgpreload_herbgrind_@VGCONF_ARCH_SEC@_@VGCONF_OS@_so_CFLAGS       = \
	$(AM_CFLAGS_PSO_@VGCONF_PLATFORM_SEC_CAPS@)
vgpreload_herbgrind_@VGCONF_ARCH_SEC@_@VGCONF_OS@_so_DEPENDENCIES =
vgpreload_herbgrind_@VGCONF_ARCH_SEC@_@VGCONF_OS@_so_LDFLAGS      = \
	$(PRELOAD_LDFLAGS_@VGCONF_PLATFORM_SEC_CAPS@)
endif
