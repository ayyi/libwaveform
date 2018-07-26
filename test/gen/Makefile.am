
lib_LTLIBRARIES = libgen.la

noinst_PROGRAMS = write_wav write_short write_block write_piano

DATA_DIR = ../data
FFMPEG = ffmpeg -y -loglevel warning

AM_CFLAGS = \
	-D_FILE_OFFSET_BITS=64 \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libs

TEST_LDFLAGS = \
	$(top_srcdir)/waveform/.libs/libwaveform.a \
	$(top_srcdir)/agl/.libs/libagl.a \
	$(top_srcdir)/transition/.libs/libtransition.a \
	$(top_srcdir)/decoder/.libs/libwaveformdecoder.a \
	-lm \
	$(GTK_LDFLAGS) \
	$(X11_LDFLAGS) \
	$(SNDFILE_LIBS) \
	$(FFMPEG_LDFLAGS) \
	$(OPENGL_LDFLAGS)

write_wav_SOURCES = \
	write_wav.cc

write_wav_LDADD = \
	$(top_srcdir)/test/gen/.libs/libgen.a \
	$(TEST_LDFLAGS)

write_short_SOURCES = \
	write_short.cc

write_short_LDADD = \
	$(top_srcdir)/test/gen/.libs/libgen.a \
	$(TEST_LDFLAGS)

write_block_SOURCES = \
	write_block.cc

write_block_LDADD = \
	$(SNDFILE_LIBS)

write_piano_SOURCES = \
	write_piano.cc \
	instrument.cc instrument.hh

write_piano_LDADD = \
	$(SNDFILE_LIBS)

libgen_la_SOURCES = \
	cpgrs.cc cpgrs.h \
	generator.cc generator.hh

BUILT_SOURCES = test_data

# disable recompilation of write_block to prevent this running each time
test_data: Makefile write_block.cc
	@make libgen.la
	@make write_wav
	@make write_short
	@make write_piano
	@if [ ! -e "write_block" ] || [ "write_block.cc" -nt "write_block" ] || [ "Makefile" -nt "write_block" ]; then \
		make write_block; \
	fi
	@if [[ ! -d $(DATA_DIR) ]]; then mkdir $(DATA_DIR); fi
	@if [ ! -e "$(DATA_DIR)/mono_0:10.wav" ] || [ "./write_wav" -nt "$(DATA_DIR)/mono_0:10.wav" ] || [ "Makefile" -nt "$(DATA_DIR)/mono_0:10.wav" ]; then \
		./write_wav $(DATA_DIR)/mono_0:10.wav; \
		$(FFMPEG) -i $(DATA_DIR)/mono_0:10.wav -q 2 $(DATA_DIR)/mono_0:10.mp3; \
		$(FFMPEG) -i $(DATA_DIR)/mono_0:10.wav -vbr 3 $(DATA_DIR)/mono_0:10.m4a; \
		$(FFMPEG) -i $(DATA_DIR)/mono_0:10.wav $(DATA_DIR)/mono_0:10.opus; \
		$(FFMPEG) -i $(DATA_DIR)/mono_0:10.wav $(DATA_DIR)/mono_0:10.flac; \
		$(FFMPEG) -i $(DATA_DIR)/mono_0:10.wav -acodec pcm_s24le $(DATA_DIR)/mono_24b_0:10.wav; \
	fi;
	@if [ ! -e "$(DATA_DIR)/stereo_0:10.wav" ] || [ "./write_wav" -nt "$(DATA_DIR)/stereo_0:10.wav" ] || [ "Makefile" -nt "$(DATA_DIR)/stereo_0:10.wav" ]; then \
		./write_wav --channels=2 $(DATA_DIR)/stereo_0:10.wav; \
		$(FFMPEG) -i $(DATA_DIR)/stereo_0:10.wav -q 2 $(DATA_DIR)/stereo_0:10.mp3; \
		$(FFMPEG) -i $(DATA_DIR)/stereo_0:10.wav -vbr 3 $(DATA_DIR)/stereo_0:10.m4a; \
		$(FFMPEG) -i $(DATA_DIR)/stereo_0:10.wav $(DATA_DIR)/stereo_0:10.opus; \
		$(FFMPEG) -i $(DATA_DIR)/stereo_0:10.wav $(DATA_DIR)/stereo_0:10.flac; \
		$(FFMPEG) -i $(DATA_DIR)/stereo_0:10.wav -acodec pcm_s24le $(DATA_DIR)/stereo_24b_0:10.wav; \
	fi;
	@if [ ! -e "$(DATA_DIR)/mono_10:00.wav" ] || [ "./write_wav" -nt "$(DATA_DIR)/mono_10:00.wav" ]; then \
		./write_wav --length=600  $(DATA_DIR)/mono_10:00.wav; \
		$(FFMPEG) -i $(DATA_DIR)/mono_10:00.wav -q 2 $(DATA_DIR)/mono_10:00.mp3; \
	fi;
	@if [ ! -e "$(DATA_DIR)/short.wav" ] || [ "./write_short" -nt "$(DATA_DIR)/short.wav" ]; then \
		./write_short $(DATA_DIR)/short.wav; \
	fi;
	@if [ ! -e "$(DATA_DIR)/1_block.wav" ] || [ "./write_block" -nt "$(DATA_DIR)/1_block.wav" ]; then \
		./write_block $(DATA_DIR)/1_block.wav; \
	fi;
	@if [ ! -e "$(DATA_DIR)/2_blocks.wav" ] || [ "./write_block" -nt "$(DATA_DIR)/2_blocks.wav" ]; then \
		./write_block --numblocks=2 $(DATA_DIR)/2_blocks.wav; \
	fi;
	@if [ ! -e "$(DATA_DIR)/3_blocks.wav" ] || [ "./write_block" -nt "$(DATA_DIR)/3_blocks.wav" ]; then \
		./write_block --numblocks=3 $(DATA_DIR)/3_blocks.wav; \
	fi;
	@if [ ! -e "$(DATA_DIR)/piano.wav" ] || [ "./write_piano" -nt "$(DATA_DIR)/piano.wav" ]; then \
		./write_piano $(DATA_DIR)/piano.wav; \
	fi;

EXTRA_DIST = \
	piano.cc piano.hh
