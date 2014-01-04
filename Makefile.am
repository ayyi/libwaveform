if ENABLE_OPENGL
OPENGL_SUBDIRS = agl shaders
endif
SUBDIRS = $(OPENGL_SUBDIRS) waveform transition test

AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"Waveform\" \
	-D_FILE_OFFSET_BITS=64 \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(includedir) \
	-I$(top_srcdir)/libs \
	$(GTK_CFLAGS) \
	$(OPENGL_CFLAGS)

SHADERS = \
	shaders/peak.vert \
	shaders/peak.frag \
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
	shaders/ass.frag

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

.PHONY: dummy

test: dummy
	cd test && make test

CLEANFILES = waveform.pc \
	shaders/shaders.c
