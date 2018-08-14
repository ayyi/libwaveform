
SUBDIRS = gdk gtk

#ACLOCAL_AMFLAGS = -I m4macros ${ACLOCAL_FLAGS}

EXTRA_DIST = \
	configure \
	gdkglext-uninstalled.pc.in \
	gtkglext-uninstalled.pc.in

## Copy .pc files to target-specific names
GDK_TARGET = @gdktarget@
API_VER = @GTKGLEXT_API_VERSION@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =
$(pkgconfig_DATA): $(top_builddir)/config.status

DISTCLEANFILES = \
	gdkglext-$(GDK_TARGET)-$(API_VER)-uninstalled.pc \
	gdkglext-$(API_VER)-uninstalled.pc \
	gtkglext-$(GDK_TARGET)-$(API_VER)-uninstalled.pc \
	gtkglext-$(API_VER)-uninstalled.pc

