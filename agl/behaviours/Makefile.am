
noinst_LTLIBRARIES = libbehaviours.la

libbehaviours_la_SOURCES = \
	selectable.c selectable.h

AM_CFLAGS = \
	-no-pie \
	-Wl,-z,noexecstack \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir) \
	$(GTK_CFLAGS)
