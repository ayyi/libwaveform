
noinst_LTLIBRARIES = libbehaviours.la

libbehaviours_la_SOURCES = \
	clip.c clip.h \
	border.c border.h \
	key.c key.h \
	selectable.c selectable.h

AM_CFLAGS = \
	-no-pie \
	-Wl,-z,noexecstack \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/agl \
	-I$(top_srcdir) \
	$(GRAPHENE_CFLAGS) \
	$(GTK_CFLAGS)
