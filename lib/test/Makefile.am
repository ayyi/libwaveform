if BUILD_TESTS

AM_CFLAGS = \
	-DG_LOG_DOMAIN=\"Waveform\" \
	-I$(top_srcdir)/lib \
	$(GLIB_CFLAGS)

noinst_LTLIBRARIES = libtest.la

libtest_la_SOURCES = \
	runner.c runner.h

endif

