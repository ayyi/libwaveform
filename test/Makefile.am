SUBDIRS = gen

if BUILD_TESTS
if ENABLE_SNDFILE

if ENABLE_LIBASS
ASSPROGRAMS = ass
endif
if ENABLE_SDL
SDLPROGRAMS = sdl
endif

if ENABLE_GTK
GTKPROGRAMS = view_plus actor actor_plus actor_no_scene list group hires res_vhi multi_scene $(ASSPROGRAMS) frame_rate resources cache pixbuf
LOCAL_GTK_LDFLAGS = $(GTK_LDFLAGS)
endif

if ENABLE_OPENGL
if !ENABLE_SYSTEM_GTKGLEXT
if ENABLE_GTK
LOCAL_GTKGLEXT_CFLAGS = \
	-I$(top_srcdir)/gtkglext-1.0
TEST_GTKGLEXT_LDFLAGS = \
	-lgmodule-2.0 \
	$(top_srcdir)/gtkglext-1.0/.libs/libgtkglext.a
endif
endif
endif

if ENABLE_OPENGL
noinst_PROGRAMS = waveform large_files promise 32bit glx input $(GTKPROGRAMS) $(SDLPROGRAMS)
else
noinst_PROGRAMS = waveform
endif

if ENABLE_EPOXY
GL_LOADER_LDFLAGS = $(EPOXY_LDFLAGS)
endif

AM_CFLAGS = \
	-fno-common \
	-D_FILE_OFFSET_BITS=64 \
	-I$(top_srcdir) \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/agl \
	$(LOCAL_GTKGLEXT_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(GTK_CFLAGS) \
	$(SDL_CFLAGS) \
	$(GRAPHENE_CFLAGS) \
	$(LIBASS_CFLAGS)

promise_CFLAGS = -D__no_setup__ $(AM_CFLAGS)

# be careful before changing the order here
TEST_LDFLAGS = \
	$(top_srcdir)/.libs/libwaveformui.a \
	$(top_srcdir)/.libs/libwaveformcore.a \
	$(top_srcdir)/lib/agl/.libs/libagl.a \
	../lib/debug/.libs/libayyidebug.a \
	../lib/test/.libs/libtest.a \
	$(OPENGL_LDFLAGS) \
	$(GL_LOADER_LDFLAGS) \
	$(LIBASS_LDFLAGS) \
	$(LOCAL_GTK_LDFLAGS) \
	$(TEST_GTKGLEXT_LDFLAGS) \
	$(GMODULE_LDFLAGS) \
	$(SNDFILE_LIBS) \
	$(FFMPEG_LDFLAGS) \
	$(SYSPROF_LIBS) \
	$(GRAPHENE_LDFLAGS) \
	$(XRANDR_LDFLAGS) \
	$(GDK_LIBS) \
	$(X11_LDFLAGS) \
	$(FRIBIDI_LDFLAGS) \
	-lm

COMMON_SOURCES = \
	common.c common.h \
	common2.c common2.h

COMMON2_SOURCES = \
	common2.c common2.h

waveform_SOURCES = \
	$(COMMON2_SOURCES) \
	waveform.c

view_plus_SOURCES = \
	$(COMMON2_SOURCES) \
	view_plus.c

actor_SOURCES = \
	$(COMMON2_SOURCES) \
	actor.c

actor_no_scene_SOURCES = \
	$(COMMON2_SOURCES) \
	actor_no_scene.c

multi_scene_SOURCES = \
	$(COMMON2_SOURCES) \
	multi_scene.c

actor_plus_SOURCES = \
	$(COMMON2_SOURCES) \
	actor_plus.c

list_SOURCES = \
	$(COMMON2_SOURCES) \
	list.c

group_SOURCES = \
	$(COMMON2_SOURCES) \
	group.c

ass_SOURCES = $(COMMON2_SOURCES) ass.c

hires_SOURCES = \
	$(COMMON2_SOURCES) \
	hires.c

res_vhi_SOURCES = \
	$(COMMON2_SOURCES) \
	res_vhi.c

frame_rate_SOURCES = \
	$(COMMON2_SOURCES) \
	frame_rate.c

resources_SOURCES = \
	$(COMMON2_SOURCES) \
	resources.c

large_files_SOURCES = \
	large_files.c

cache_SOURCES = \
	$(COMMON_SOURCES) \
	cache.c

32bit_SOURCES = \
	$(COMMON_SOURCES) \
	32bit.c

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

input_SOURCES = \
	$(COMMON2_SOURCES) \
	input.c

#promise_SOURCES = promise.c

waveform_LDADD = \
	$(TEST_LDFLAGS)

view_plus_LDADD = \
	$(ass_LDADD)

actor_LDADD = \
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
	../lib/agl/sdl/libsdlframerate.a \
	$(TEST_LDFLAGS)

glx_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

input_LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

LDADD = \
	$(OPENGL_LDFLAGS) \
	$(TEST_LDFLAGS)

view_plus_DEPENDENCIES = \
	../.libs/libwaveformcore.a \
	../.libs/libwaveformui.a \
	../lib/agl/.libs/libagl.a

endif
endif

AM_TESTS_FD_REDIRECT = 9>&2 --non-interactive

AM_TESTS_ENVIRONMENT = \
	export NON_INTERACTIVE=1;

BUILT_SOURCES = waveform.h

define build_header =
	echo > $@
	echo '#include "wf/waveform.h"' >> $@
	echo '#include "test/runner.h"' >> $@
	echo >> $@
	@echo "TestFn x" >> $@
	@cat $(@:.h=.c) | grep ^test_ | sed 's/\(.*\) .*/  ,\1/' >> $@
	@echo "  ;" >> $@
	@echo >> $@
	@echo "gpointer tests[] = {" >> $@
	@cat $(@:.h=.c) | grep ^test_ | sed 's/\(.*\) .*/  \1,/' >> $@
	@echo "};" >> $@
	@echo >> $@
	@echo "int n_tests () { return "`cat $(@:.h=.c) | grep ^test_ | wc -l`"; }" >> $@
endef

waveform.h: waveform.c Makefile
	@$(build_header)

# these tests will be run as part of make-check
# TODO dont swallow stdout
TESTS = \
	waveform \
	32bit \
	cache \
	view_plus \
	actor \
	actor_plus \
	hires \
	res_vhi \
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
	./waveform
	cd .. && test/large_files
	./32bit
	./cache
	./view_plus -n
	./actor -n
	./actor_plus -n
	./hires -n
	./res_vhi -n
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
	sdl \
	view_plus \
	waveform

EXTRA_DIST = 

