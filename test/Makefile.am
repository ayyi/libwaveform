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
noinst_PROGRAMS = waveform_test large_files promise 32bit glx scenegraph text input $(GTKPROGRAMS) $(SDLPROGRAMS)
else
noinst_PROGRAMS = waveform_test
endif

if ENABLE_EPOXY
GL_LOADER_LDFLAGS = $(EPOXY_LDFLAGS)
endif

AM_CFLAGS = \
	-D_FILE_OFFSET_BITS=64 \
	-I$(top_srcdir) \
	$(LOCAL_GTKGLEXT_CFLAGS) \
	$(GTKGLEXT_LIBS) \
	$(OPENGL_CFLAGS) \
	$(GTK_CFLAGS) \
	$(SDL_CFLAGS) \
	$(GRAPHENE_CFLAGS) \
	$(LIBASS_CFLAGS)

# be careful before changing the order here
TEST_LDFLAGS = \
	$(top_srcdir)/.libs/libwaveformui.a \
	$(top_srcdir)/.libs/libwaveformcore.a \
	$(OPENGL_LDFLAGS) \
	$(GL_LOADER_LDFLAGS) \
	$(LIBASS_LDFLAGS) \
	$(GTK_LDFLAGS) \
	$(X11_LDFLAGS) \
	$(GTKGLEXT_LDFLAGS) \
	$(GMODULE_LDFLAGS) \
	$(SNDFILE_LIBS) \
	$(FFMPEG_LDFLAGS) \
	$(SYSPROF_LIBS) \
	$(GRAPHENE_LDFLAGS) \
	-lm

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

text_SOURCES = \
	$(COMMON2_SOURCES) \
	text.c

input_SOURCES = \
	$(COMMON2_SOURCES) \
	input.c

scenegraph_SOURCES = \
	$(COMMON2_SOURCES) \
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

text_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

input_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

scenegraph_LDADD = \
	$(TEST_LDFLAGS)

LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

endif

AM_TESTS_FD_REDIRECT = 9>&2 --non-interactive

AM_TESTS_ENVIRONMENT = \
	export NON_INTERACTIVE=1;

# these tests will be run as part of make-check
# TODO dont swallow stdout
TESTS = \
	waveform_test \
	32bit \
	cache \
	view_plus_test \
	actor_test \
	actor_plus \
	hires \
	scenegraph \
	viewport \
	group \
	actor_no_scene \
	multi_scene \
	frame_rate \
	glx \
	list \
	sdl \
	ass

test:
	@echo running tests...
	./waveform_test
	cd .. && test/large_files
	./32bit
	./cache
	./view_plus_test -n
	./actor_test -n
	./actor_plus -n
	./hires -n
	./scenegraph -n
	./viewport -n
	./group -n
	./actor_no_scene -n
	./multi_scene -n
	./frame_rate -n
	./glx -n
	./list -n
	./sdl -n
	./ass -n

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

