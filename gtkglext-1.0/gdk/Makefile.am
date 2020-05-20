
SUBDIRS =
DIST_SUBDIRS =

common_includes = \
	-DG_LOG_DOMAIN=\"GdkGLExt\"	\
	-DGDK_GL_COMPILATION \
	-I$(top_srcdir) \
	$(GTKGLEXT_DEBUG_FLAGS) \
	$(GDK_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GL_CFLAGS) \
	$(GDKGLEXT_WIN_CFLAGS)

common_ldflags = \
	-version-info $(GTKGLEXT_CURRENT):$(GTKGLEXT_REVISION):$(GTKGLEXT_AGE) \
	-export-dynamic \
	-rpath $(libdir) \
	-no-undefined

common_libadd = \
	$(GDK_LIBS) $(GLU_LIBS)

#
# setup source file variables
#

gdkglext_public_h_sources = \
	gdkgl.h \
	gdkgltokens.h \
	gdkgltypes.h \
	gdkglinit.h \
	gdkglquery.h \
	gdkglconfig.h \
	gdkglcontext.h \
	gdkgldrawable.h \
	gdkglpixmap.h \
	gdkglwindow.h \
	gdkglx.h \
	gdkglglxext.h

gdkglext_private_h_sources = \
	gdkglprivate.h

gdkglext_built_public_h_sources = \
	gdkglenumtypes.h

gdkglext_c_sources = \
	gdkglinit.c \
	gdkglquery.c \
	gdkglconfig.c \
	gdkglcontext.c \
	gdkgldrawable.c \
	gdkglpixmap.c \
	gdkglwindow.c \
	gdkglglxext.c

gdkglext_built_c_sources = \
	gdkglenumtypes.c

gdkglext_headers = \
	$(gdkglext_public_h_sources) \
	$(gdkglext_built_public_h_sources) \
	$(gdkglext_configure_generated_public_h_source) \
	gdkgldebug.h

gdkglext_sources = \
	$(gdkglext_private_h_sources) \
	$(gdkglext_c_sources) \
	$(gdkglext_built_c_sources)

gdkglext_built_sources = \
	$(gdkglext_built_public_h_sources) \
	$(gdkglext_built_c_sources)

stamp_files = \
	stamp-gdkglenumtypes-h

#
# setup GdkGLExt sources and their dependancies
#

gdkglextincludedir = $(includedir)/gtkglext-@GTKGLEXT_API_VERSION@/gdk
gdkglextinclude_HEADERS = $(gdkglext_headers)

AM_CPPFLAGS = $(common_includes)

BUILT_SOURCES = \
	$(gdkglext_built_sources)

DISTCLEANFILES = $(stamp_files) $(gdkglext_configure_generated_public_h_source)

MAINTAINERCLEANFILES = \
	$(gdkglext_built_sources)

# Generate built header without using automake BUILT_SOURCES
$(libgdkglext_x11_1_0_la_OBJECTS): $(gdkglext_built_public_h_sources)

$(srcdir)/gdkglenumtypes.h: $(srcdir)/gtkglenumtypes.h.template $(gdkglext_public_h_sources)
	( cd $(srcdir) && $(GLIB_MKENUMS) --template gtkglenumtypes.h.template \
$(gdkglext_public_h_sources) ) >> xgen-geth \
&& (cmp -s xgen-geth $(srcdir)/gdkglenumtypes.h || cp xgen-geth $(srcdir)/gdkglenumtypes.h ) \
&& rm -f xgen-geth

$(srcdir)/gdkglenumtypes.c: $(srcdir)/gtkglenumtypes.c.template $(gdkglext_public_h_sources)
	( cd $(srcdir) && $(GLIB_MKENUMS) --template gtkglenumtypes.c.template \
$(gdkglext_public_h_sources) ) > xgen-getc \
&& cp xgen-getc $(srcdir)/gdkglenumtypes.c  \
&& rm -f xgen-getc

#
# Rule to install gdkglext-config.h header file
#
configexecincludedir = $(libdir)/gtkglext-@GTKGLEXT_API_VERSION@/include

gdkglext-config.h: stamp-gdkglext-config-h
	@if test -f gdkglext-config.h; then :; \
	else rm -f stamp-gdkglext-config-h; $(MAKE) stamp-gdkglext-config-h; fi

stamp-gdkglext-config-h: $(top_builddir)/config.status
	cd $(top_builddir) && $(SHELL) ./config.status gdk/gdkglext-config.h
	echo timestamp > stamp-gdkglext-config-h

DISTCLEANFILES += gdkglext-config.h stamp-gdkglext-config-h

EXTRA_DIST = \
	glext/README			\
	glext/glxext.h.patch		\
	glext/common-header.h \
	gtkglenumtypes.h.template \
	gtkglenumtypes.c.template

gdkglext_glext_public_h_sources = \
	glext/glxext.h \
	glext/glext-extra.h \
	glext/glxext-extra.h \
	glext/wglext-extra.h

gdkglextglextincludedir = $(includedir)/gtkglext-@GTKGLEXT_API_VERSION@/gdk/glext
gdkglextglextinclude_HEADERS = $(gdkglext_glext_public_h_sources)

noinst_LTLIBRARIES = libgdk.la
libgdk_la_SOURCES = $(gdkglext_sources)
