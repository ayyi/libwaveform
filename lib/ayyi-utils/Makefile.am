
AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"AyyiUtils\" \
	-D_FILE_OFFSET_BITS=64 \
	-I.. \
	$(GLIB_CFLAGS)

lib_LTLIBRARIES = libayyiutils.la libpromise.la

libayyiutils_la_SOURCES = \
	value.h \
	observable.c observable.h

libpromise_la_SOURCES = \
	promise.c promise.h

pkgincludedir=$(includedir)/ayyi-utils
pkginclude_HEADERS = \
	value.h \
	observable.h \
	utils.h \
	promise.h

EXTRA_DIST = \
	utils.h

