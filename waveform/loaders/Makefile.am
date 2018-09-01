
noinst_LTLIBRARIES = libloaders.la

libloaders_la_SOURCES = \
	ardour.c ardour.h \
	riff.c riff.h

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

pkgincludedir=$(includedir)/waveform/loaders
pkginclude_HEADERS = \
	ardour.h \
	riff.h
