
CLEANFILES =

EXTRA_DIST =

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
	$(gtkglext_public_h_sources) \
	gtkgldebug.h

gtkglext_sources = \
	$(gtkglext_private_h_sources) \
	$(gtkglext_c_sources)

#
# setup GtkGLExt sources and their dependancies
#

gtkglextincludedir = $(includedir)/gtkglext-@GTKGLEXT_API_VERSION@/gtk
gtkglextinclude_HEADERS = $(gtkglext_headers)

AM_CPPFLAGS = $(common_includes)

#libgtkglext_x11_1_0_la_SOURCES = $(gtkglext_sources)
#libgtkglext_x11_1_0_la_LDFLAGS = $(common_ldflags)
#libgtkglext_x11_1_0_la_LIBADD = $(common_libadd)

noinst_LTLIBRARIES = libgtk.la
libgtk_la_SOURCES = $(gtkglext_sources)
