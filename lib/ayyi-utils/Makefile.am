
AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"AyyiUtils\" \
	-D_FILE_OFFSET_BITS=64 \
	-I.. \
	$(GLIB_CFLAGS)

lib_LTLIBRARIES = libayyiutils.la

libayyiutils_la_SOURCES = \
	observable.c observable.h

pkgincludedir=$(includedir)/ayyi-utils
pkginclude_HEADERS = \
	observable.h \
	utils.h

BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST = \
	utils.h

