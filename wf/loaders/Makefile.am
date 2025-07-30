
noinst_LTLIBRARIES = libloaders.la

libloaders_la_SOURCES = \
	ardour.c ardour.h \
	riff.c riff.h

if ENABLE_SYSTEM_GTKGLEXT
else
LOCAL_GTKGLEXT_CFLAGS = \
	-I$(top_srcdir)/gtkglext-1.0
endif

AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"Waveform\" \
	-D_FILE_OFFSET_BITS=64 \
	-I$(includedir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/lib \
	$(GTK_CFLAGS) \
	$(SDL_CFLAGS) \
	$(LOCAL_GTKGLEXT_CFLAGS) \
	$(OPENGL_CFLAGS)

pkgincludedir=$(includedir)/waveform/loaders
pkginclude_HEADERS = \
	ardour.h \
	riff.h
