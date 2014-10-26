SUBDIRS =

INCLUDES = \
	-DG_LOG_DOMAIN=\"Transition\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(includedir) \
	$(GTK_CFLAGS)

lib_LTLIBRARIES = libtransition.la

libtransition_la_SOURCES = \
	transition.c transition.h \
	globals.c \
	frameclock.c frameclock.h frameclockprivate.h \
	frameclockidle.c frameclockidle.h \
	frametimings.c frametimings.h

pkgincludedir=$(includedir)/transition
pkginclude_HEADERS = \
	transition.h \
	frameclock.h frameclockprivate.h frameclockidle.h frametimings.h

BUILT_SOURCES =

CLEANFILES =

EXTRA_DIST =

