
if BUILD_TESTS

if ENABLE_LIBASS
ASSPROGRAMS = ass
endif
if ENABLE_SDL
SDLPROGRAMS = sdl
endif

if ENABLE_OPENGL
noinst_PROGRAMS = waveform_test view_test view_plus_test actor_test actor_plus actor_no_scene list group hires_test multi_scene $(ASSPROGRAMS) frame_rate resources write_wav write_short write_block large_files cache 32bit pixbuf glx scenegraph viewport $(SDLPROGRAMS)
###noinst_PROGRAMS = waveform_test view_test actor_test actor_plus $(ASSPROGRAMS)
endif

AM_CFLAGS = \
	-O0 \
	-D_FILE_OFFSET_BITS=64 \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libs \
	$(OPENGL_CFLAGS) \
	$(GTK_CFLAGS) \
	$(LIBASS_CFLAGS)

TEST_LDFLAGS = \
	$(top_srcdir)/waveform/.libs/libwaveform.a \
	$(top_srcdir)/agl/.libs/libagl.a \
	$(top_srcdir)/transition/.libs/libtransition.a \
	$(top_srcdir)/decoder/.libs/libwaveformdecoder.a \
	-lm \
	$(GTK_LDFLAGS) \
	$(SNDFILE_LIBS) \
	$(FFMPEG_LDFLAGS) \
	$(OPENGL_LDFLAGS)

COMMON_SOURCES = \
	common.c common.h \
	common2.c common2.h \
	ayyi_utils.c ayyi_utils.h

COMMON2_SOURCES = \
	common2.c common2.h \
	ayyi_utils.c ayyi_utils.h

waveform_test_SOURCES = \
	$(COMMON_SOURCES) \
	waveform_test.c

view_test_SOURCES = \
	$(COMMON_SOURCES) \
	view_test.c

view_plus_test_SOURCES = \
	$(COMMON_SOURCES) \
	view_plus_test.c

actor_test_SOURCES = \
	$(COMMON_SOURCES) \
	actor_test.c

actor_no_scene_SOURCES = \
	$(COMMON_SOURCES) \
	actor_no_scene.c

multi_scene_SOURCES = \
	$(COMMON_SOURCES) \
	multi_scene.c

actor_plus_SOURCES = \
	$(COMMON_SOURCES) \
	actor_plus.c

list_SOURCES = \
	$(COMMON_SOURCES) \
	list.c

group_SOURCES = \
	$(COMMON_SOURCES) \
	group.c

ass_SOURCES = $(COMMON_SOURCES) ass.c

hires_test_SOURCES = \
	ayyi_utils.c ayyi_utils.h \
	hires_test.c

frame_rate_SOURCES = \
	$(COMMON2_SOURCES) \
	frame_rate.c

resources_SOURCES = \
	$(COMMON2_SOURCES) \
	resources.c

large_files_SOURCES = \
	$(COMMON_SOURCES) \
	large_files.c

cache_SOURCES = \
	$(COMMON_SOURCES) \
	cache.c

32bit_SOURCES = \
	$(COMMON_SOURCES) \
	32bit.c

#med_textures_SOURCES = \
#	$(COMMON_SOURCES) \
#	med_textures.c

pixbuf_SOURCES = \
	$(COMMON_SOURCES) \
	pixbuf.c

if ENABLE_SDL
sdl_SOURCES = \
	$(COMMON_SOURCES) \
	sdl.c
endif

glx_sources = \
	$(COMMON_SOURCES) \
	glx.c

scenegraph_sources = \
	$(COMMON_SOURCES) \
	scenegraph.c

viewport_SOURCES = \
	$(COMMON_SOURCES) \
	viewport.c

write_wav_SOURCES = \
	generator.cc generator.h \
	cpgrs.cc cpgrs.h \
	write_wav.cpp

write_short_SOURCES = \
	generator.cc generator.h \
	cpgrs.cc cpgrs.h \
	write_short.cc

write_block_SOURCES = \
	write_block.cc

waveform_test_LDADD = \
	$(TEST_LDFLAGS)

view_test_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

view_plus_test_LDADD = \
	$(ass_LDADD)

actor_test_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

actor_no_scene_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

actor_plus_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

ass_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(LIBASS_LDFLAGS) \
	$(TEST_LDFLAGS)

sdl_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(SDL_LIBS) \
	$(top_srcdir)/sdl/libsdlframerate.a \
	$(TEST_LDFLAGS)

glx_LDADD = \
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

# disable recompilation of write_block to prevent this running each time
test_data: Makefile write_block.cc
	@make write_wav
	@make write_short
	@make write_block
	@if [[ ! -d data ]]; then mkdir data; fi
	@if [ ! -e "data/mono_0:10.wav" ] || [ "./write_wav" -nt "data/mono_0:10.wav" ] || [ "Makefile" -nt "data/mono_0:10.wav" ]; then \
		./write_wav data/mono_0:10.wav; \
		ffmpeg -y -i data/mono_0:10.wav -q 2 data/mono_0:10.mp3; \
		ffmpeg -y -i data/mono_0:10.wav -vbr 3 data/mono_0:10.m4a; \
		ffmpeg -y -i data/mono_0:10.wav data/mono_0:10.opus; \
		ffmpeg -y -i data/mono_0:10.wav data/mono_0:10.flac; \
	fi;
	@if [ ! -e "data/stereo_0:10.wav" ] || [ "./write_wav" -nt "data/stereo_0:10.wav" ] || [ "Makefile" -nt "data/stereo_0:10.wav" ]; then \
		./write_wav --channels=2 data/stereo_0:10.wav; \
		ffmpeg -y -i data/stereo_0:10.wav -q 2 data/stereo_0:10.mp3; \
		ffmpeg -y -i data/stereo_0:10.wav -vbr 3 data/stereo_0:10.m4a; \
		ffmpeg -y -i data/stereo_0:10.wav data/stereo_0:10.opus; \
		ffmpeg -y -i data/stereo_0:10.wav data/stereo_0:10.flac; \
	fi;
	@if [ ! -e "data/mono_10:00.wav" ] || [ "./write_wav" -nt "data/mono_10:00.wav" ]; then \
		./write_wav --length=600  data/mono_10:00.wav; \
		ffmpeg -y -i data/mono_10:00.wav -q 2 data/mono_10:00.mp3; \
	fi;
	@if [ ! -e "data/short.wav" ] || [ "./write_short" -nt "data/short.wav" ]; then \
		./write_short data/short.wav; \
	fi;
	@if [ ! -e "data/1_block.wav" ] || [ "./write_block" -nt "data/1_block.wav" ]; then \
		./write_block data/1_block.wav; \
	fi;
	@if [ ! -e "data/2_blocks.wav" ] || [ "./write_block" -nt "data/2_blocks.wav" ]; then \
		./write_block --numblocks=2 data/2_blocks.wav; \
	fi;
	@if [ ! -e "data/3_blocks.wav" ] || [ "./write_block" -nt "data/3_blocks.wav" ]; then \
		./write_block --numblocks=3 data/3_blocks.wav; \
	fi;

endif

test:
	@echo running tests...
	cd .. && test/waveform_test
	cd .. && test/large_files
	cd .. && test/32bit
	cd .. && test/cache

EXTRA_DIST = 

