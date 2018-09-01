
noinst_LTLIBRARIES = libactors.la

libactors_la_SOURCES = \
	group.c group.h \
	background.c background.h \
	plain.c plain.h \
	ruler.c ruler.h \
	grid.c grid.h \
	spp.c spp.h \
	spinner.c spinner.h \
	text.c text.h \
	debug.c debug.h

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
	$(GTK_CFLAGS) \
	$(LIBASS_CFLAGS) \
	$(SDL_CFLAGS) \
	$(LOCAL_GTKGLEXT_CFLAGS) \
	$(OPENGL_CFLAGS)

pkgincludedir=$(includedir)/waveform/actors
pkginclude_HEADERS = \
	background.h \
	plain.h \
	debug.h \
	grid.h \
	group.h \
	ruler.h \
	spinner.h \
	spp.h \
	text.h
