if ENABLE_OPENGL
OPENGL_SUBDIRS = agl shaders
endif
if ENABLE_SDL
SDL_SUBDIRS = sdl
endif
if ENABLE_GTK
GTK_SUBDIRS = gtkglext-1.0
endif
if ENABLE_GLIB_2_58_POLYFILL
GLIB_SUBDIRS = glib
GLIB_LIBADD = glib/libglib.la
endif
SUBDIRS = $(GTK_SUBDIRS) $(OPENGL_SUBDIRS) $(SDL_SUBDIRS) $(GLIB_SUBDIRS) wf ui transition decoder . test

if ENABLE_OPENGL
OPENGL_LIBADD = \
	agl/libagl.la \
	agl/behaviours/libbehaviours.la
endif

lib_LTLIBRARIES = libwaveformui.la libwaveformcore.la

libwaveformcore_la_SOURCES =
libwaveformcore_la_LIBADD = \
	decoder/libdecoder.la \
	wf/libwfcore.la

libwaveformui_la_SOURCES =
libwaveformui_la_LIBADD = \
	transition/libtransition.la \
	$(GLIB_LIBADD) \
	ui/libwf.la \
	$(OPENGL_LIBADD)

AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"Waveform\" \
	-D_FILE_OFFSET_BITS=64 \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(top_srcdir)/libs \
	$(GTK_CFLAGS) \
	$(OPENGL_CFLAGS)

SHADERS = \
	shaders/peak.vert \
	shaders/peak.frag \
	shaders/hires_ng.vert \
	shaders/hires_ng.frag \
	shaders/vertical.vert \
	shaders/vertical.frag \
	shaders/horizontal.vert \
	shaders/horizontal.frag \
	shaders/hires.vert \
	shaders/hires.frag \
	shaders/peak_nonscaling.vert \
	shaders/peak_nonscaling.frag \
	shaders/ruler.vert \
	shaders/ruler.frag \
	shaders/lines.vert \
	shaders/lines.frag \
	shaders/ass.vert \
	shaders/ass.frag \
	shaders/cursor.vert \
	shaders/cursor.frag

EXTRA_DIST = \
	$(SHADERS) \
	shaders/Makefile \
	shaders/make_shaders \
	package/gentoo/media-libs/libwaveform/libwaveform-0.1.0.ebuild

shaderdir = $(datadir)/$(PACKAGE)/shaders

## shaders are now compiled in, so no longer need to be installed
Xinstall-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(shaderdir); \
	for shader in shaders/peak.vert shaders/peak.frag shaders/vertical.vert shaders/vertical.frag $(SHADERS); do \
		if test -f $$shader; then \
			$(INSTALL_DATA) $$shader $(DESTDIR)$(shaderdir); \
		fi \
	done;

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libwaveform.pc

.PHONY: dummy

CLEANFILES = libwaveform.pc \
	shaders/shaders.c
