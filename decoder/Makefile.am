
INCLUDES = \
	-DG_LOG_DOMAIN=\"AGl\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(includedir) \
	-I$(top_srcdir)/waveform \
	$(GTK_CFLAGS)

lib_LTLIBRARIES = libwaveformdecoder.la

if ENABLE_FFMPEG
FF_SRC = \
	ffmpeg.c
endif

libwaveformdecoder_la_SOURCES = \
	ad.c ad.h \
	sndfile.c \
	debug.h \
	$(FF_SRC)

pkgincludedir=$(includedir)/decoder

pkginclude_HEADERS = \
	ad.h

