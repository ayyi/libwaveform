SUBDIRS =

AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"Waveform\" \
	-D_FILE_OFFSET_BITS=64 \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(includedir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(GTK_CFLAGS) \
	$(OPENGL_CFLAGS)

lib_LTLIBRARIES = libwaveform.la

if ENABLE_OPENGL
OPENGL_SOURCES = \
	view.c view.h \
	canvas.c canvas.h \
	actor.c actor.h \
	gl_utils.c gl_utils.h \
	shader.c shader.h \
	alphabuf.c alphabuf.h \
	grid.c grid.h \
	animator.c animator.h \
	texture_cache.c texture_cache.h \
	hi_res.c hi_res.h \
	fbo.c fbo.h \
	gl_ext.c gl_ext.h
endif

libwaveform_la_SOURCES = \
	typedefs.h waveform.h wf_private.h \
	global.c global.h \
	peak.c peak.h \
	loaders/ardour.c loaders/ardour.h \
	loaders/riff.c loaders/riff.h \
	shaderutil.c shaderutil.h \
	$(OPENGL_SOURCES) \
	peakgen.c peakgen.h \
	audio.c audio.h \
	utils.c utils.h

EXTRA_DIST = \
	view_gl.c

pkginclude_HEADERS = \
	waveform.h \
	peak.h \
	peakgen.h \
	loaders/ardour.h \
	loaders/riff.h \
	view.h \
	actor.h \
	canvas.h \
	hi_res.h \
	utils.h \
	shaderutil.h \
	gl_utils.h \
	gl_ext.h \
	wf_private.h

CLEANFILES =

