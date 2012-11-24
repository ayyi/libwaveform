INCLUDES = \
	-DG_LOG_DOMAIN=\"AGl\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(includedir) \
	-I$(top_srcdir)/waveform \
	$(OPENGL_CFLAGS) \
	$(GTK_CFLAGS)

lib_LTLIBRARIES = libagl.la

libagl_la_SOURCES = \
	fontmap.c fontmap.h \
	pango_render.c pango_render.h \
	pango_font.c pango_font.h \
	ext.c ext.h \
	fbo.h \
	utils.c utils.h

pkginclude_HEADERS = \
	ext.h

BUILT_SOURCES =

CLEANFILES =

EXTRA_DIST =

