SUBDIRS = loaders actors

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

noinst_LTLIBRARIES = libwf.la

if ENABLE_GTK
GTK_SOURCES = \
	view.c view.h \
	view_plus.c view_plus.h
endif

if ENABLE_OPENGL
OPENGL_SOURCES = \
	$(GTK_SOURCES) \
	context.c context.h \
	actor.c actor.h \
	shader.c shader.h \
	texture_cache.c texture_cache.h \
	fbo.c fbo.h
OPENGL_LIBADD = \
	actors/libactors.la
endif

libwf_la_SOURCES = \
	typedefs.h waveform.h wf_private.h \
	global.c global.h \
	waveform.c waveform.h \
	$(OPENGL_SOURCES) \
	alphabuf.c alphabuf.h \
	peakgen.c peakgen.h \
	audio.c audio.h \
	worker.c worker.h \
	promise.c promise.h \
	utils.c utils.h

libwf_la_LIBADD = \
	loaders/libloaders.la \
	$(OPENGL_LIBADD)

EXTRA_DIST = \
	renderer/ng.c \
	renderer/res_med.c \
	renderer/res_hi.c \
	renderer/res_hi_gl2.c \
	renderer/res_v_hi.c \
	renderer/res_low.c \
	renderer/res_v_low.c

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

