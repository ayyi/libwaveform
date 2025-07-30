SUBDIRS = actors

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
	-I../lib/agl \
	$(GTK_CFLAGS) \
	$(SDL_CFLAGS) \
	$(LOCAL_GTKGLEXT_CFLAGS) \
	$(OPENGL_CFLAGS)

noinst_LTLIBRARIES = libwf.la

if ENABLE_GTK
GTK_SOURCES = \
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

if ENABLE_DEBUG
DEBUG_SOURCES = \
	debug_helper.c debug_helper.h
endif

libwf_la_SOURCES = \
	typedefs.h \
	private.h \
	$(OPENGL_SOURCES) \
	pixbuf.c pixbuf.h \
	transition_behaviour.c transition_behaviour.h \
	invalidator.c invalidator.h \
	utils.c utils.h \
	$(DEBUG_SOURCES)

libwf_la_LIBADD = \
	$(OPENGL_LIBADD)

EXTRA_DIST = \
	renderer/ng.c \
	renderer/res_med.c \
	renderer/res_hi.c \
	renderer/res_hi_gl2.c \
	renderer/res_v_hi.c \
	renderer/res_lo.c \
	renderer/res_v_low.c

CLEANFILES =

