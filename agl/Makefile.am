SUBDIRS = shaders behaviours text

GLIB_COMPILE_RESOURCES = $(shell $(PKG_CONFIG) --variable=glib_compile_resources gio-2.0)

BUILT_SOURCES = resources.c

green = "\033[1;32m"
normal = "\033[0;39m"

resources.c: gresource.xml resources/shaders/color.glsl resources/shaders/coloring.glsl resources/shaders/preamble.glsl resources/shaders/preamble.fs.glsl
	@echo -en $(green)
	@echo compiling resources ...
	@echo -en $(normal)
	$(GLIB_COMPILE_RESOURCES) gresource.xml --target=$@ --sourcedir=. --generate-source

if ENABLE_SYSTEM_GTKGLEXT
else
LOCAL_GTKGLEXT_CFLAGS = \
	-I$(top_srcdir)/gtkglext-1.0
endif

AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"AGl\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(includedir) \
	-I$(top_srcdir)/waveform \
	$(LOCAL_GTKGLEXT_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(SDL_CFLAGS) \
	$(GRAPHENE_CFLAGS) \
	$(GTK_CFLAGS)

noinst_LTLIBRARIES = libagl.la

libagl_la_SOURCES = \
	ext.c ext.h \
	fbo.c fbo.h \
	actor.c actor.h \
	utils.c utils.h \
	material.c material.h \
	transform.c transform.h \
	behaviour.h \
	observable.c observable.h \
	typedefs.h shader.h \
	resources.c

libagl_la_LIBADD = \
	text/libtext.la

pkgincludedir=$(includedir)/agl
pkginclude_HEADERS = \
	typedefs.h \
	ext.h \
	fbo.h \
	shader.h \
	material.h \
	actor.h \
	utils.h \
	behaviour.h \
	text/pango.h

CLEANFILES =

SHADERS = \
	shaders/alpha_map.vert shaders/alpha_map.frag \
	shaders/plain_colour.vert shaders/plain_colour.frag \
	shaders/texture_2d.vert shaders/texture_2d.frag

EXTRA_DIST = \
	debug.h \
	$(SHADERS) \
	shaders/Makefile \
	shaders/make_shaders

