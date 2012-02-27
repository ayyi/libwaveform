SUBDIRS = waveform test

AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"Waveform\" \
	-D_FILE_OFFSET_BITS=64 \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(includedir) \
	-I$(top_srcdir)/libs \
	$(GTK_CFLAGS) \
	$(OPENGL_CFLAGS)

EXTRA_DIST = \
	shaders/peak.vert \
	shaders/peak.frag \
	package/gentoo/media-libs/libwaveform/libwaveform-0.1.0.ebuild

shaderdir = $(datadir)/$(PACKAGE)/shaders

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(shaderdir); \
	for shader in shaders/peak.vert shaders/peak.frag; do \
		if test -f $$shader; then \
			$(INSTALL_DATA) $$shader $(DESTDIR)$(shaderdir); \
		fi \
	done;
