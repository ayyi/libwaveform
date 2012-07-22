
if BUILD_TESTS

noinst_PROGRAMS = waveform_test view_test actor_test actor_plus hires_test frame_rate resources write_wav write_short

AM_CFLAGS = \
	-O0 \
	-D_FILE_OFFSET_BITS=64 \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libs \
	$(OPENGL_CFLAGS) \
	$(GTK_CFLAGS)

TEST_LDFLAGS = \
	$(top_srcdir)/waveform/.libs/libwaveform.a \
	$(GTK_LDFLAGS) \
	$(SNDFILE_LIBS) \
	$(OPENGL_LDFLAGS)

waveform_test_SOURCES = \
	common.c common.h \
	ayyi_utils.c ayyi_utils.h \
	waveform_test.c

view_test_SOURCES = \
	ayyi_utils.c ayyi_utils.h \
	common.c common.h \
	view_test.c

actor_test_SOURCES = \
	ayyi_utils.c ayyi_utils.h \
	actor_test.c

actor_plus_SOURCES = \
	ayyi_utils.c ayyi_utils.h \
	actor_plus.c

hires_test_SOURCES = \
	ayyi_utils.c ayyi_utils.h \
	hires_test.c

frame_rate_SOURCES = \
	ayyi_utils.c ayyi_utils.h \
	frame_rate.c

resources_SOURCES = \
	ayyi_utils.c ayyi_utils.h \
	resources.c

write_wav_SOURCES = \
	cpgrs.cc cpgrs.h \
	write_wav.cpp

write_short_SOURCES = \
	cpgrs.cc cpgrs.h \
	write_short.cc

waveform_test_LDADD = \
	$(TEST_LDFLAGS)

view_test_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

actor_test_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

actor_plus_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

write_wav_LDADD = \
	$(TEST_LDFLAGS)

write_short_LDADD = \
	$(TEST_LDFLAGS)

BUILT_SOURCES = test_data

test_data: Makefile
	@make write_wav
	@make write_short
	@if [[ ! -d data ]]; then mkdir data; fi
	@if [ ! -e "data/mono_1.wav" ] || [ "./write_wav" -nt "data/mono_1.wav" ]; then \
		./write_wav data/mono_1.wav; \
	fi;
	@if [ ! -e "data/stereo_1.wav" ] || [ "./write_wav" -nt "data/stereo_1.wav" ]; then \
		./write_wav --channels=2 data/stereo_1.wav; \
	fi;
	@if [ ! -e "data/short.wav" ] || [ "./write_short" -nt "data/short.wav" ]; then \
		./write_short data/short.wav; \
	fi;

endif

EXTRA_DIST =

