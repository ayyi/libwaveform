all: *.vert *.frag
	../lib/agl/agl/shaders/make_shaders peak peak_nonscaling horizontal vertical hires hires_ng ruler ruler_bottom ruler_frames ass lines cursor

clean-local:
	if [ -f shaders.c ]; then rm shaders.c; fi

SHADERS = \
	peak.vert \
	peak.frag \
	hires_ng.vert \
	hires_ng.frag \
	vertical.vert \
	vertical.frag \
	horizontal.vert \
	horizontal.frag \
	hires.vert \
	hires.frag \
	peak_nonscaling.vert \
	peak_nonscaling.frag \
	ruler.vert \
	ruler.frag \
	ruler_bottom.vert \
	ruler_bottom.frag \
	ruler_frames.vert \
	ruler_frames.frag \
	lines.vert \
	lines.frag \
	ass.vert \
	ass.frag \
	cursor.vert \
	cursor.frag

EXTRA_DIST = \
	$(SHADERS)

## shaders are now compiled in, so no longer need to be installed
Xinstall-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(shaderdir); \
	for shader in shaders/peak.vert shaders/peak.frag shaders/vertical.vert shaders/vertical.frag $(SHADERS); do \
		if test -f $$shader; then \
			$(INSTALL_DATA) $$shader $(DESTDIR)$(shaderdir); \
		fi \
	done;

