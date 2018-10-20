
noinst_LTLIBRARIES = libdecoder.la

AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"AGl\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(includedir) \
	$(GLIB_CFLAGS)

if ENABLE_FFMPEG
FF_SRC = \
	ffmpeg.c
endif

libdecoder_la_SOURCES = \
	ad.c ad.h \
	sndfile.c \
	debug.h \
	$(FF_SRC)

pkgincludedir=$(includedir)/decoder

pkginclude_HEADERS = \
	ad.h

