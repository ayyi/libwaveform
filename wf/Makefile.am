SUBDIRS = loaders

AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"Waveform\" \
	-D_FILE_OFFSET_BITS=64 \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(includedir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/lib \
	$(FFMPEG_CFLAGS) \
	$(GLIB_CFLAGS)

noinst_LTLIBRARIES = libwfcore.la

libwfcore_la_SOURCES = \
	typedefs.h \
	private.h \
	global.c \
	waveform.c waveform.h \
	peakgen.c peakgen.h \
	audio.c audio.h \
	worker.c worker.h \
	promise.c promise.h \
	utils.c utils.h \
	debug.h

libwfcore_la_LIBADD = \
	loaders/libloaders.la

