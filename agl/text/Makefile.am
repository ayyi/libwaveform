NULL =

CLEANFILES =
DISTCLEANFILES =

if ENABLE_PROFILER
PROFILER_SRC = \
	profiler.c profiler.h
endif

source_c = \
	types.h \
	text_node.c text_node.h \
	text_input.c text_input.h \
	text_buffer.c text_buffer.h \
	pango.c pango.h \
	glyphcache.c glyphcache.h \
	textureatlas.c textureatlas.h \
	renderops.c renderops.h \
	opbuffer.c opbuffer.h \
	stb_rect_pack.c stb_rect_pack.h \
	shaderbuilder.c shaderbuilder.h \
	gdkrgba.c gdkrgba.h \
	renderer.c renderer.h \
	driver.c driver.h \
	roundedrect.c roundedrect.h \
	$(PROFILER_SRC) \
	$(NULL)

source_h_priv = \
	$(NULL)

if ENABLE_SYSTEM_GTKGLEXT
else
LOCAL_GTKGLEXT_CFLAGS = \
	-I$(top_srcdir)/gtkglext-1.0
endif

noinst_LTLIBRARIES = libtext.la

libtext_la_SOURCES = $(source_c) $(source_h_priv)

libtext_la_CFLAGS = \
	-DG_LOG_DOMAIN=\"AGltext\" \
	-I$(top_srcdir)/agl \
	$(LOCAL_GTKGLEXT_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GRAPHENE_CFLAGS) \
	$(SYSPROF_CFLAGS) \
	$(MAINTAINER_CFLAGS)

libtext_la_LDFLAGS = \
	-export-dynamic \
	-no-undefined

EXTRA_DIST = \
	enums.h

