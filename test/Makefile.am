SUBDIRS = gen

if BUILD_TESTS
if ENABLE_SNDFILE

if ENABLE_SDL
SDLPROGRAMS = sdl
endif

if ENABLE_GTK
GTKPROGRAMS = view_plus actor actor_plus actor_no_scene list group hires res_vhi res_vhi_blocks res_vhi_scrollable multi_scene frame_rate resources cache pixbuf
LOCAL_GTK_LDFLAGS = $(GTK_LDFLAGS)
endif

if ENABLE_OPENGL
if !ENABLE_SYSTEM_GTKGLEXT
if ENABLE_GTK
if ENABLE_GTK3
LOCAL_GTKGLEXT_CFLAGS = \
	-I$(top_srcdir)/lib/agl/lib/gtkglext3
LOCAL_GTKGLEXT_LDFLAGS = \
	$(GTKGLEXT_LIBS)
else
LOCAL_GTKGLEXT_CFLAGS = \
	-I$(top_srcdir)/lib/agl/gtkglext-1.0
LOCAL_GTKGLEXT_LDFLAGS = \
	$(top_srcdir)/lib/agl/gtkglext-1.0/.libs/libgtkglext.a
endif
TEST_GTKGLEXT_LDFLAGS = \
	$(GMODULE_LDFLAGS) \
	$(LOCAL_GTKGLEXT_LDFLAGS)
endif
endif
endif

if ENABLE_OPENGL
noinst_PROGRAMS = waveform large-files promise 32bit unit-actor glx input preview $(GTKPROGRAMS) $(SDLPROGRAMS)
else
noinst_PROGRAMS = waveform preview
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
	$(GRAPHENE_CFLAGS)

promise_CFLAGS = -D__no_setup__ $(AM_CFLAGS)

NONGUI_SYSTEM_LDFLAGS = \
	$(LOCAL_GTK_LDFLAGS) \
	$(GMODULE_LDFLAGS) \
	$(SNDFILE_LIBS) \
	$(FFMPEG_LDFLAGS) \
	-lm

# be careful before changing the order here
TEST_LDFLAGS = \
	$(top_srcdir)/.libs/libwaveformui.a \
	$(top_srcdir)/.libs/libwaveformcore.a \
	$(top_srcdir)/lib/agl/.libs/libagl.a \
	$(top_srcdir)/lib/debug/.libs/libayyidebug.a \
	$(top_srcdir)/lib/ayyi-utils/.libs/libayyiutils.a \
	$(top_srcdir)/lib/test/.libs/libtest.a \
	$(OPENGL_LDFLAGS) \
	$(GL_LOADER_LDFLAGS) \
	$(TEST_GTKGLEXT_LDFLAGS) \
	$(SYSPROF_LIBS) \
	$(GRAPHENE_LDFLAGS) \
	$(XRANDR_LDFLAGS) \
	$(GDK_LIBS) \
	$(NONGUI_SYSTEM_LDFLAGS) \
	$(X11_LDFLAGS) \
	$(FRIBIDI_LDFLAGS)

NONGUI_TEST_LDFLAGS = \
	$(top_srcdir)/.libs/libwaveformcore.a \
	$(top_srcdir)/lib/debug/.libs/libayyidebug.a \
	$(top_srcdir)/lib/test/.libs/libtest.a \
	../lib/ayyi-utils/.libs/libayyiutils.a \
	$(NONGUI_SYSTEM_LDFLAGS)

COMMON_SOURCES = \
	common.c common.h \
	common2.c common2.h

COMMON2_SOURCES = \
	common2.c common2.h

waveform_SOURCES = \
	$(COMMON2_SOURCES) \
	waveform.c

unit_actor_SOURCES = \
	$(COMMON2_SOURCES) \
	unit-actor.c

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
	large-files.c

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

large_files_LDADD = \
	$(NONGUI_TEST_LDFLAGS)

sdl_LDADD = \
	$(SDL_LIBS) \
	../lib/agl/sdl/libsdlframerate.a \
	$(TEST_LDFLAGS)

LDADD = \
	$(TEST_LDFLAGS)

res_vhi_blocks_LDADD = \
	$(LDADD) \
	common2.o

res_vhi_scrollable_LDADD = \
	$(LDADD) \
	common2.o

preview_LDADD = \
	$(NONGUI_TEST_LDFLAGS)

view_plus_DEPENDENCIES = \
	../.libs/libwaveformcore.a \
	../.libs/libwaveformui.a \
	../lib/agl/.libs/libagl.a

endif
endif

AM_TESTS_FD_REDIRECT = 9>&2 --non-interactive

AM_TESTS_ENVIRONMENT = \
	export NON_INTERACTIVE=1;

BUILT_SOURCES = waveform.h 32bit.h promise.h unit-actor.h large-files.h

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

32bit.h: 32bit.c Makefile
	@$(build_header)

waveform.h: waveform.c Makefile
	@$(build_header)

promise.h: promise.c Makefile
	@$(build_header)

unit-actor.h: unit-actor.c Makefile
	@$(build_header)

large-files.h: large-files.c Makefile
	@$(build_header)

# these tests will be run as part of make-check
TESTS = \
	waveform \
	preview \
	32bit \
	unit-actor \
	cache \
	view_plus \
	actor \
	actor_plus \
	hires \
	res_vhi \
	res_vhi_blocks \
	res_vhi_scrollable \
	group \
	actor_no_scene \
	multi_scene \
	frame_rate \
	glx \
	list \
	sdl

test:
	@echo running tests...
	./waveform
	./preview
	cd .. && test/large-files
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

CLEANFILES = \
	large-files \
	list \
	multi_scene \
	pixbuf \
	resources \
	sdl \
	view_plus \
	waveform

EXTRA_DIST = 

valgrind:
	export G_SLICE=always-malloc && \
	export G_DEBUG=gc-friendly && \
	/usr/bin/valgrind \
		--leak-check=full \
		--gen-suppressions=yes \
		--num-callers=50 \
		--suppressions=./suppressions/test.supp \
		./waveform

.PHONY: valgrind
