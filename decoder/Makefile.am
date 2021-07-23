
noinst_LTLIBRARIES = libdecoder.la

AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"Decoder\" \
	-I$(includedir) \
	$(FFMPEG_CFLAGS) \
	$(GLIB_CFLAGS)

if ENABLE_FFMPEG
FF_SRC = \
	ffmpeg.c
endif

if ENABLE_SNDFILE
SNDFILE_SRC = \
	sndfile.c
endif

libdecoder_la_SOURCES = \
	ad.c ad.h \
	$(SNDFILE_SRC) \
	$(FF_SRC) \
	debug.h

pkgincludedir=$(includedir)/decoder

pkginclude_HEADERS = \
	ad.h

