SUBDIRS = shaders

INCLUDES = \
	-DG_LOG_DOMAIN=\"AGl\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(includedir) \
	-I$(top_srcdir)/waveform \
	$(OPENGL_CFLAGS) \
	$(SDL_CFLAGS) \
	$(GTK_CFLAGS)

lib_LTLIBRARIES = libagl.la

libagl_la_SOURCES = \
	fontmap.c fontmap.h \
	pango_render.c pango_render.h \
	pango_font.c pango_font.h \
	ext.c ext.h \
	fbo.c fbo.h \
	actor.c actor.h \
	utils.c utils.h \
	material.c material.h \
	typedefs.h shader.h

pkgincludedir=$(includedir)/agl
pkginclude_HEADERS = \
	typedefs.h \
	ext.h \
	fbo.h \
	shader.h \
	actor.h \
	utils.h \
	pango_render.h \
	fontmap.h

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

