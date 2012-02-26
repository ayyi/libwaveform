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

noinst_LIBRARIES = libwaveform.a

CLEANFILES =

libwaveform_a_SOURCES = \
	typedefs.h waveform.h wf_private.h \
	peak.c peak.h \
	loaders/ardour.c loaders/ardour.h \
	loaders/riff.c loaders/riff.h \
	gl_utils.c gl_utils.h \
	shaderutil.c shaderutil.h \
	view.c view.h \
	canvas.c canvas.h \
	actor.c actor.h \
	grid.c grid.h \
	animator.c animator.h \
	peakgen.c peakgen.h \
	texture_cache.c texture_cache.h \
	hi_res.c hi_res.h \
	audio.c audio.h \
	gl_ext.c gl_ext.h \
	utils.c utils.h

EXTRA_DIST = \
	view_gl.c
