if BUILD_TESTS

lib_LTLIBRARIES = libgen.la

noinst_PROGRAMS = write_wav write_short write_block write_piano thumbnail

DATA_DIR = ../data
FFMPEG = ffmpeg -y -loglevel warning

AM_CFLAGS = \
	-D_FILE_OFFSET_BITS=64

if ENABLE_GTK
GTKGLEXT_LDFLAGS = \
	-lgmodule-2.0
endif

TEST_LDFLAGS = \
	$(top_srcdir)/.libs/libwaveformcore.a \
	-lm \
	$(GTK_LDFLAGS) \
	$(X11_LDFLAGS) \
	$(SNDFILE_LIBS) \
	$(FFMPEG_LDFLAGS) \
	$(OPENGL_LDFLAGS) \
	$(GTKGLEXT_LDFLAGS)

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

thumbnail_SOURCES = \
	thumbnail.c

thumbnail_CFLAGS = \
	$(GTK_CFLAGS)

thumbnail_LDADD = \
	$(GTK_LDFLAGS)

libgen_la_SOURCES = \
	cpgrs.cc cpgrs.h \
	generator.cc generator.h

BUILT_SOURCES = test_data

# disable recompilation of write_block to prevent this running each time
test_data: Makefile libgen.la test_data_1 test_data_thumbnail

test_data_1: write_block.cc write_wav write_short write_piano
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
		$(FFMPEG) -i $(DATA_DIR)/mono_0:10.wav -acodec pcm_u8 $(DATA_DIR)/mono_8b_0:10.wav; \
		./write_wav -l 0; \
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
		./write_short -l 10 $(DATA_DIR)/v_short.wav; \
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

test_data_thumbnail: thumbnail
	./thumbnail
	ffmpeg -i $(DATA_DIR)/mono_0:10.mp3 -i thumbnail.png -map 0:0 -map 1:0 -codec copy -id3v2_version 3 \
	    -metadata:s:v title="Thumbnail" -metadata:s:v comment="Thumbnail" -y $(DATA_DIR)/thumbnail.mp3
	rm thumbnail.png

clean-local:
	if [ -f thumbnail.png ]; then rm thumbnail.png; fi
	if [ -f $(DATA_DIR)/thumbnail.mp3 ]; then rm $(DATA_DIR)/thumbnail.mp3; fi
	if [ -f $(DATA_DIR)/mono_0:10.mp3 ]; then rm $(DATA_DIR)/mono_0:10.mp3; fi
	if [ -f $(DATA_DIR)/mono_10:00.wav ]; then rm $(DATA_DIR)/mono_10:00.wav; fi
	if [ -f $(DATA_DIR)/large1.wav ]; then rm $(DATA_DIR)/large1.wav; fi
	if [ -f $(DATA_DIR)/large2.wav ]; then rm $(DATA_DIR)/large2.wav; fi
	if [ -f $(DATA_DIR)/stereo_24b_0:10.wav ]; then rm $(DATA_DIR)/stereo_24b_0:10.wav; fi

endif

EXTRA_DIST = \
	piano.cc piano.hh
