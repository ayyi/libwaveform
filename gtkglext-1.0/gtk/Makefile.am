
CLEANFILES =

EXTRA_DIST = \
	gtkglversion.h

common_includes = \
	-DG_LOG_DOMAIN=\"GtkGLExt\"	\
	-DGTK_GL_COMPILATION \
	-I$(top_srcdir) \
	-I$(top_builddir)/gdk \
	$(GTKGLEXT_DEBUG_FLAGS) \
	$(GTK_CFLAGS) \
	$(GL_CFLAGS) \
	$(GDKGLEXT_WIN_CFLAGS)

common_ldflags = \
	-version-info $(GTKGLEXT_CURRENT):$(GTKGLEXT_REVISION):$(GTKGLEXT_AGE) \
	-export-dynamic \
	-rpath $(libdir) \
	-no-undefined

common_libadd = \
	$(top_builddir)/gdk/$(gdkglext_targetlib) \
	$(GTK_LIBS) \
	$(GL_LIBS) \
	$(GDKGLEXT_WIN_LIBS) \
	$(MATH_LIB)

#
# setup source file variables
#

gtkglext_public_h_sources = \
	gtkgl.h \
	gtkglinit.h	\
	gtkglwidget.h

gtkglext_private_h_sources = \
	gtkglprivate.h

gtkglext_c_sources = \
	gtkglinit.c	\
	gtkglwidget.c

gtkglext_headers = \
	$(gtkglext_public_h_sources)	\
	gtkgldebug.h

gtkglext_sources = \
	$(gtkglext_private_h_sources)	\
	$(gtkglext_c_sources)

#
# setup GtkGLExt sources and their dependancies
#

gtkglextincludedir = $(includedir)/gtkglext-@GTKGLEXT_API_VERSION@/gtk
gtkglextinclude_HEADERS = $(gtkglext_headers)

AM_CPPFLAGS = $(common_includes)

lib_LTLIBRARIES = $(gtkglext_targetlib)

EXTRA_LTLIBRARIES = \
	libgtkglext-x11-@API_MJ@.@API_MI@.la	\
	libgtkglext-quartz-@API_MJ@.@API_MI@.la

libgtkglext_x11_@API_MJ@_@API_MI@_la_SOURCES = $(gtkglext_sources)
libgtkglext_x11_@API_MJ@_@API_MI@_la_LDFLAGS = $(common_ldflags)
libgtkglext_x11_@API_MJ@_@API_MI@_la_LIBADD = $(common_libadd)

libgtkglext_quartz_@API_MJ@_@API_MI@_la_SOURCES = $(gtkglext_sources)
libgtkglext_quartz_@API_MJ@_@API_MI@_la_LDFLAGS = $(common_ldflags)
libgtkglext_quartz_@API_MJ@_@API_MI@_la_LIBADD = $(common_libadd)


