SUBDIRS =

AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"Transition\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(includedir) \
	$(GTK_CFLAGS)

noinst_LTLIBRARIES = libtransition.la

libtransition_la_SOURCES = \
	transition.c transition.h \
	frameclock.c frameclock.h frameclockprivate.h \
	frameclockidle.c frameclockidle.h \
	frametimings.c frametimings.h \
	globals.c

pkgincludedir=$(includedir)/transition
pkginclude_HEADERS = \
	transition.h \
	frameclock.h frameclockprivate.h frameclockidle.h frametimings.h

BUILT_SOURCES =

CLEANFILES =

EXTRA_DIST =

