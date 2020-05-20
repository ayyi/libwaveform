CLEANFILES =
DISTCLEANFILES =

if ENABLE_GLIB_2_58_POLYFILL
GLIB_SRC = \
	grcbox.c grcbox.h \
	garcbox.c \
	grefcount.c grefcount.h \
	grcboxprivate.h
endif

noinst_LTLIBRARIES = libglib.la

libglib_la_SOURCES = \
	$(GLIB_SRC)

libglib_la_CFLAGS = \
	-DG_LOG_DOMAIN=\"GLib\" \
	$(GTK_CFLAGS) \
	$(MAINTAINER_CFLAGS)

libglib_la_LDFLAGS = \
	-export-dynamic \
	-no-undefined

EXTRA_DIST =

