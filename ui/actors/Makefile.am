
if ENABLE_OPENGL

noinst_LTLIBRARIES = libactors.la

libactors_la_SOURCES = \
	ruler.c ruler.h \
	grid.c grid.h \
	labels.c labels.h \
	spp.c spp.h \
	text.c text.h

if ENABLE_SYSTEM_GTKGLEXT
else
LOCAL_GTKGLEXT_CFLAGS = \
	-I$(top_srcdir)/gtkglext-1.0
endif

AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"Waveform\" \
	-D_FILE_OFFSET_BITS=64 \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(includedir) \
	-I$(top_srcdir) \
	-I../../lib/agl \
	$(GTK_CFLAGS) \
	$(LIBASS_CFLAGS) \
	$(SDL_CFLAGS) \
	$(LOCAL_GTKGLEXT_CFLAGS) \
	$(OPENGL_CFLAGS)

endif

pkgincludedir=$(includedir)/waveform/actors
pkginclude_HEADERS = \
	grid.h \
	labels.h \
	ruler.h \
	spp.h \
	text.h
