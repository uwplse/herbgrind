extra_deps_@VGCONF_ARCH_PRI@ = \
	$(top_builddir)/../mpfr/install-@VGCONF_ARCH_PRI@/lib/libmpfr.a \
	$(top_builddir)/../gmp/install-@VGCONF_ARCH_PRI@/lib/libgmp.a

extra_deps_@VGCONF_ARCH_SEC@ = \
	$(top_builddir)/../mpfr/install-@VGCONF_ARCH_SEC@/lib/libmpfr.a \
	$(top_builddir)/../gmp/install-@VGCONF_ARCH_SEC@/lib/libgmp.a

include $(top_srcdir)/Makefile.tool.am

EXTRA_DIST = docs/hg-manual.xml

#----------------------------------------------------------------------------
# herbgrind-<platform>
#----------------------------------------------------------------------------

noinst_PROGRAMS  = herbgrind-@VGCONF_ARCH_PRI@-@VGCONF_OS@
if VGCONF_HAVE_PLATFORM_SEC
noinst_PROGRAMS += herbgrind-@VGCONF_ARCH_SEC@-@VGCONF_OS@
endif

HERBGRIND_SOURCES_COMMON = hg_main.c

herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_SOURCES      = \
	$(HERBGRIND_SOURCES_COMMON)
herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_CPPFLAGS     = \
	$(AM_CPPFLAGS_@VGCONF_PLATFORM_PRI_CAPS@)
herbgrind_@VGCONF_ARCH_PRI@_@VGCONF_OS@_CFLAGS       = \
	$(AM_CFLAGS_@VGCONF_PLATFORM_PRI_CAPS@) \
	-g \
	-I$(top_srcdir)/../gmp/install-@VGCONF_ARCH_PRI@/include/ \
	-I$(top_srcdir)/../mpfr/install-@VGCONF_ARCH_PRI@/include/
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
	-I$(top_srcdir)/../gmp/install-@VGCONF_ARCH_SEC@/include/ \
	-I$(top_srcdir)/../mpfr/install-@VGCONF_ARCH_SEC@/include/
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


