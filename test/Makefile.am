SUBDIRS = gen

if BUILD_TESTS

if ENABLE_LIBASS
ASSPROGRAMS = ass
endif
if ENABLE_SDL
SDLPROGRAMS = sdl
endif
if ENABLE_GTK
GTKPROGRAMS = view_plus_test actor_test actor_plus actor_no_scene list group hires multi_scene $(ASSPROGRAMS) frame_rate resources cache viewport pixbuf
endif

if ENABLE_OPENGL
if !ENABLE_SYSTEM_GTKGLEXT
LOCAL_GTKGLEXT_CFLAGS = \
	-I$(top_srcdir)/gtkglext-1.0
GTKGLEXT_LDFLAGS = \
	-lgmodule-2.0 \
	$(top_srcdir)/gtkglext-1.0/.libs/libgtkglext.a
GTKGLEXT_LIBS =
endif
endif

if ENABLE_OPENGL
noinst_PROGRAMS = waveform_test $(GTKPROGRAMS) large_files promise 32bit glx scenegraph $(SDLPROGRAMS)
###noinst_PROGRAMS = waveform_test view_plus_test actor_test actor_plus $(ASSPROGRAMS)
else
noinst_PROGRAMS = waveform_test
endif

AM_CFLAGS = \
	-D_FILE_OFFSET_BITS=64 \
	-I$(top_srcdir) \
	$(LOCAL_GTKGLEXT_CFLAGS) \
	$(GTKGLEXT_LIBS) \
	$(OPENGL_CFLAGS) \
	$(GTK_CFLAGS) \
	$(SDL_CFLAGS) \
	$(LIBASS_CFLAGS)

TEST_LDFLAGS = \
	-lm \
	$(GTK_LDFLAGS) \
	$(X11_LDFLAGS) \
	$(SNDFILE_LIBS) \
	$(FFMPEG_LDFLAGS) \
	$(OPENGL_LDFLAGS) \
	$(top_srcdir)/libwaveform.la \
	$(GTKGLEXT_LDFLAGS)

COMMON_SOURCES = \
	common.c common.h \
	common2.c common2.h

COMMON2_SOURCES = \
	common2.c common2.h

waveform_test_SOURCES = \
	$(COMMON_SOURCES) \
	waveform_test.c

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

hires_SOURCES = \
	$(COMMON_SOURCES) \
	hires.c

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

glx_SOURCES = \
	$(COMMON2_SOURCES) \
	glx.c

scenegraph_sources = \
	$(COMMON_SOURCES) \
	scenegraph.c

viewport_SOURCES = \
	$(COMMON2_SOURCES) \
	viewport.c

promise_SOURCES = \
	$(COMMON_SOURCES) \
	promise.c

waveform_test_LDADD = \
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

endif

test:
	@echo running tests...
	cd .. && test/waveform_test
	cd .. && test/large_files
	cd .. && test/32bit
	cd .. && test/cache

CLEANFILES = \
	large_files \
	list \
	multi_scene \
	pixbuf \
	resources \
	scenegraph \
	sdl \
	view_plus_test \
	viewport \
	waveform_test

EXTRA_DIST = 

