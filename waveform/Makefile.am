SUBDIRS = loaders actors

if ENABLE_LIBASS
ASS_SOURCES = view_plus.c view_plus.h
endif

AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"Waveform\" \
	-D_FILE_OFFSET_BITS=64 \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(includedir) \
	-I$(top_srcdir) \
	$(GTK_CFLAGS) \
	$(LIBASS_CFLAGS) \
	$(OPENGL_CFLAGS)

lib_LTLIBRARIES = libwaveform.la

if ENABLE_OPENGL
OPENGL_SOURCES = \
	view.c view.h \
	context.c context.h \
	actor.c actor.h \
	shader.c shader.h \
	alphabuf.c alphabuf.h \
	texture_cache.c texture_cache.h \
	fbo.c fbo.h \
	actors/group.c actors/group.h \
	actors/background.c actors/background.h \
	actors/ruler.c actors/ruler.h \
	actors/grid.c actors/grid.h \
	actors/spp.c actors/spp.h \
	actors/text.c actors/text.h \
	$(ASS_SOURCES)
endif

if ENABLE_FFMPEG
AUDIOFILE_SOURCES = \
	audio_file.c audio_file.h
endif

libwaveform_la_SOURCES = \
	typedefs.h waveform.h wf_private.h \
	global.c global.h \
	waveform.c waveform.h \
	loaders/ardour.c loaders/ardour.h \
	loaders/riff.c loaders/riff.h \
	$(OPENGL_SOURCES) \
	peakgen.c peakgen.h \
	$(AUDIOFILE_SOURCES) \
	audio.c audio.h \
	worker.c worker.h \
	promise.c promise.h \
	utils.c utils.h

EXTRA_DIST = \
	renderer_ng.c \
	med_res.c \
	hi_res.c \
	hi_res_gl2.c \
	v_hi_res.c \
	v_low_res.c

pkgincludedir=$(includedir)/waveform
pkginclude_HEADERS = \
	waveform.h \
	typedefs.h \
	peakgen.h \
	view.h \
	view_plus.h \
	actor.h \
	context.h \
	shader.h \
	fbo.h \
	texture_cache.h \
	alphabuf.h \
	promise.h \
	utils.h \
	wf_private.h

CLEANFILES =

