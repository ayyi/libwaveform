
lib_LTLIBRARIES = libgen.la

noinst_PROGRAMS = write_piano

AM_CFLAGS = \
	-D_FILE_OFFSET_BITS=64 \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libs

write_piano_SOURCES = \
	write_piano.cc \
	instrument.cc instrument.hh

write_piano_LDADD = \
	$(SNDFILE_LIBS)

libgen_la_SOURCES = \
	cpgrs.cc cpgrs.h \
	generator.cc generator.hh \
	brass.cc

