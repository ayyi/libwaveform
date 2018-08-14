
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
	gdkglglext.h \
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
	gdkglglext.c \
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

lib_LTLIBRARIES = $(gdkglext_targetlib)

EXTRA_LTLIBRARIES = \
	libgdkglext-x11-@API_MJ@.@API_MI@.la \
	libgdkglext-quartz-@API_MJ@.@API_MI@.la \
	libgdkglext-win32-@API_MJ@.@API_MI@.la

libgdkglext_x11_@API_MJ@_@API_MI@_la_SOURCES = $(gdkglext_sources)
libgdkglext_x11_@API_MJ@_@API_MI@_la_LDFLAGS = $(common_ldflags)

libgdkglext_quartz_@API_MJ@_@API_MI@_la_SOURCES = $(gdkglext_sources)
libgdkglext_quartz_@API_MJ@_@API_MI@_la_LDFLAGS = $(common_ldflags)

libgdkglext_win32_@API_MJ@_@API_MI@_la_SOURCES = $(gdkglext_sources)
libgdkglext_win32_@API_MJ@_@API_MI@_la_LDFLAGS = $(common_ldflags)
libgdkglext_win32_@API_MJ@_@API_MI@_la_DEPENDENCIES = $(gdkglext_def)

BUILT_SOURCES = \
	$(gdkglext_built_sources)

DISTCLEANFILES = $(stamp_files) $(gdkglext_configure_generated_public_h_source)

MAINTAINERCLEANFILES = \
	$(gdkglext_built_sources)

# Generate built header without using automake BUILT_SOURCES
$(libgdkglext_x11_@API_MJ@_@API_MI@_la_OBJECTS) $(libgdkglext_quartz_@API_MJ@_@API_MI@_la_OBJECTS) $(libgdkglext_win32_@API_MJ@_@API_MI@_la_OBJECTS): $(gdkglext_built_public_h_sources)

$(srcdir)/gdkglenumtypes.h: $(srcdir)/gtkglenumtypes.h.template $(gdkglext_public_h_sources) $(abs_builddir)/$(gdkglext_configure_generated_public_h_source)
	( cd $(srcdir) && $(GLIB_MKENUMS) --template gtkglenumtypes.h.template \
$(gdkglext_public_h_sources) $(abs_builddir)/$(gdkglext_configure_generated_public_h_source) ) >> xgen-geth \
&& (cmp -s xgen-geth $(srcdir)/gdkglenumtypes.h || cp xgen-geth $(srcdir)/gdkglenumtypes.h ) \
&& rm -f xgen-geth

$(srcdir)/gdkglenumtypes.c: $(srcdir)/gtkglenumtypes.c.template $(gdkglext_public_h_sources) $(abs_builddir)/$(gdkglext_configure_generated_public_h_source)
	( cd $(srcdir) && $(GLIB_MKENUMS) --template gtkglenumtypes.c.template \
$(gdkglext_public_h_sources) $(abs_builddir)/$(gdkglext_configure_generated_public_h_source) ) > xgen-getc \
&& cp xgen-getc $(srcdir)/gdkglenumtypes.c  \
&& rm -f xgen-getc

#
# Rule to install gdkglext-config.h header file
#
configexecincludedir = $(libdir)/gtkglext-@GTKGLEXT_API_VERSION@/include
configexecinclude_DATA = gdkglext-config.h

install-exec-local: gdkglext-config.h
	$(mkinstalldirs) $(DESTDIR)$(configexecincludedir)
	file=$(DESTDIR)$(configexecincludedir)/gdkglext-config.h; \
	if test -r $$file && cmp -s gdkglext-config.h $$file; then :; \
	else $(INSTALL_DATA) gdkglext-config.h $$file; fi

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
	glext/list-ext.pl		\
	glext/gen-gdkglglext-c.pl	\
	glext/gen-gdkglglext-h.pl	\
	glext/gen-gdkglglxext-c.pl	\
	glext/gen-gdkglglxext-h.pl	\
	glext/gen-gdkglwglext-c.pl	\
	glext/gen-gdkglwglext-h.pl	\
	glext/common-header.h \
	gtkglenumtypes.h.template \
	gtkglenumtypes.c.template

gdkglext_glext_public_h_sources = \
	glext/glext.h \
	glext/glxext.h \
	glext/wglext.h \
	glext/glext-extra.h \
	glext/glxext-extra.h \
	glext/wglext-extra.h

gdkglextglextincludedir = $(includedir)/gtkglext-@GTKGLEXT_API_VERSION@/gdk/glext
gdkglextglextinclude_HEADERS = $(gdkglext_glext_public_h_sources)

