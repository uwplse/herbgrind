extra_deps_amd64 = \
	$(top_builddir)/../deps/mpfr-64/install/lib/libmpfr.a \
	$(top_builddir)/../deps/gmp-64/install/lib/libgmp.a

extra_deps_i386 = \
	$(top_builddir)/../deps/mpfr-32/install/lib/libmpfr.a \
	$(top_builddir)/../deps/gmp-32/install/lib/libgmp.a

include $(top_srcdir)/Makefile.tool.am

EXTRA_DIST = docs/hg-manual.xml

#----------------------------------------------------------------------------
# herbgrind-<platform>
#----------------------------------------------------------------------------

noinst_PROGRAMS  = herbgrind-@VGCONF_ARCH_PRI@-@VGCONF_OS@
if VGCONF_HAVE_PLATFORM_SEC
noinst_PROGRAMS += herbgrind-@VGCONF_ARCH_SEC@-@VGCONF_OS@
endif

HERBGRIND_SOURCES_COMMON = hg_main.c hg_runtime.c hg_instrument.c	\
hg_evaluate.c hg_types.c

herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_SOURCES      = \
	$(HERBGRIND_SOURCES_COMMON)
herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_CPPFLAGS     = \
	$(AM_CPPFLAGS_@VGCONF_PLATFORM_PRI_CAPS@)
herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_CFLAGS       = \
	$(AM_CFLAGS_@VGCONF_PLATFORM_PRI_CAPS@) \
	-g \
	-I$(top_srcdir)/../deps/gmp-64/install/include/ \
	-I$(top_srcdir)/../deps/mpfr-64/install/include/
herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_DEPENDENCIES = \
	$(TOOL_DEPENDENCIES_@VGCONF_PLATFORM_PRI_CAPS@)
herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_LDADD        = \
	$(TOOL_LDADD_@VGCONF_PLATFORM_PRI_CAPS@)
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
	-g \
	-I$(top_srcdir)/../deps/gmp-32/install/include/ \
	-I$(top_srcdir)/../deps/mpfr-32/install/include/
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


