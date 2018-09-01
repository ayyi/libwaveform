
SUBDIRS = gdk gtk

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

lib_LTLIBRARIES = libgtkglext.la
libgtkglext_la_SOURCES =
libgtkglext_la_LIBADD = \
  gdk/libgdk.la \
  gtk/libgtk.la
