
if BUILD_TESTS

bin_PROGRAMS = waveform_test view_test actor_test frame_rate resources write_wav

AM_CFLAGS = \
	-O0 \
	-D_FILE_OFFSET_BITS=64 \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libs \
	$(OPENGL_CFLAGS) \
	$(GTK_CFLAGS)

TEST_LDFLAGS = \
	$(top_srcdir)/waveform/libwaveform.a \
	$(GTK_LDFLAGS) \
	$(SNDFILE_LIBS)

waveform_test_SOURCES = \
	common.c common.h \
	ayyi_utils.c ayyi_utils.h \
	waveform_test.c

view_test_SOURCES = \
	ayyi_utils.c ayyi_utils.h \
	view_test.c

actor_test_SOURCES = \
	ayyi_utils.c ayyi_utils.h \
	actor_test.c

frame_rate_SOURCES = \
	ayyi_utils.c ayyi_utils.h \
	frame_rate.c

resources_SOURCES = \
	ayyi_utils.c ayyi_utils.h \
	resources.c

write_wav_SOURCES = \
	write_wav.cpp

waveform_test_LDADD = \
	$(TEST_LDFLAGS)

view_test_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

actor_test_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

write_wav_LDADD = \
	$(TEST_LDFLAGS)

test_data: Makefile
	mkdir data
	./write_wav data/mono_1.wav
	./write_wav --channels=2 data/stereo_1.wav

endif

EXTRA_DIST =

