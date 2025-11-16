if ENABLE_OPENGL
OPENGL_SUBDIRS = shaders
endif
if ENABLE_GTK
GTK_SUBDIRS = gtkglext-1.0
endif
SUBDIRS = $(GTK_SUBDIRS) lib $(OPENGL_SUBDIRS) wf ui decoder . test waveform

lib_LTLIBRARIES = libwaveformui.la libwaveformcore.la

libwaveformcore_la_SOURCES =
libwaveformcore_la_LIBADD = \
	decoder/libdecoder.la \
	wf/libwfcore.la

libwaveformui_la_SOURCES =
libwaveformui_la_LIBADD = \
	ui/libwf.la

AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"Waveform\" \
	-D_FILE_OFFSET_BITS=64 \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(top_srcdir)/libs \
	$(GTK_CFLAGS) \
	$(OPENGL_CFLAGS)

EXTRA_DIST = \
	package/gentoo/media-libs/libwaveform/libwaveform-0.1.0.ebuild

shaderdir = $(datadir)/$(PACKAGE)/shaders

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libwaveform.pc

.PHONY: dummy

CLEANFILES = libwaveform.pc
