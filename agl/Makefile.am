SUBDIRS = shaders

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
	fbo.c fbo.h \
	utils.c utils.h \
	typedefs.h shader.h

pkginclude_HEADERS = \
	ext.h

BUILT_SOURCES =

CLEANFILES =

SHADERS = \
	shaders/alpha_map.vert shaders/alpha_map.frag \
	shaders/plain_colour.vert shaders/plain_colour.frag \
	shaders/texture_2d.vert shaders/texture_2d.frag

EXTRA_DIST = \
	$(SHADERS) \
	shaders/Makefile \
	shaders/make_shaders

