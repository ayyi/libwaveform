AC_PREREQ(2.59)
AC_INIT([Libwaveform],[0.1.0],[tim@orford.org])
AC_CONFIG_SRCDIR([waveform/waveform.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE
AM_PROG_AR

# LT_REVISION=1

AC_PROG_CXX
AC_PROG_CC_C99
AC_C_CONST
AC_C_INLINE
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_CONFIG_MACRO_DIRS([m4])

# Library building stuff
AC_PROG_RANLIB
AC_PROG_LIBTOOL

AX_SUBDIRS_CONFIGURE([gtkglext-1.0], [[--enable-test=false]], [], [])

# At least level 2 optimisation is needed to allow GCC to
# determine that not all nested functions require trampolines.
CFLAGS="$CFLAGS -O2"

# Check for debugging flag
debug="yes"
AC_ARG_ENABLE(debug,
	[AS_HELP_STRING(--enable-debug, [Enable debugging (false)])],
	[debug="$enableval"])
if test "$debug" = "yes"; then
  CFLAGS="$CFLAGS -g -DDEBUG -Wall"
  CXXFLAGS="$CFLAGS"
else
  CFLAGS="$CFLAGS -DNDEBUG"
  CXXFLAGS="$CXXFLAGS -DNDEBUG"
fi

# Check for strict flag
strict="no"
AC_ARG_ENABLE(strict,
	[AS_HELP_STRING(--enable-strict, [Enable strict compiler warnings and errors (false)])],
	[strict="$enableval"])
if test "$strict" = "yes"; then
  CFLAGS="$CFLAGS -Wall -Wextra -Wno-unused-parameter -Wconversion -Winit-self"
  CXXFLAGS="$CXXFLAGS -Wall -Wextra -Wno-unused-parameter -Wconversion -Winit-self -Woverloaded-virtual -Wsign-promo"
fi

JACK_LIBS=`pkg-config jack --cflags --libs`
AC_SUBST(JACK_LIBS)

SNDFILE_LIBS=`pkg-config --cflags --libs sndfile`
AC_SUBST(SNDFILE_LIBS)

AYYI_CFLAGS="-I./ayyi"
AC_SUBST(AYYI_CFLAGS)
GLIB_CFLAGS=`pkg-config --cflags glib-2.0`
AC_SUBST(GLIB_CFLAGS)
GTHREAD_LIBS=`pkg-config --libs gthread-2.0`
AC_SUBST(GTHREAD_LIBS)

X11_LDFLAGS=`pkg-config --libs x11`
AC_SUBST(X11_LDFLAGS)

GMODULE_LDFLAGS=`pkg-config gmodule-2.0 --libs`
AC_SUBST(GMODULE_LDFLAGS)

dnl -----------------------------------------------------

GTK_CFLAGS=`pkg-config gtk+-2.0 --cflags`
GTK_LDFLAGS=`pkg-config gtk+-2.0 --libs`
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LDFLAGS)

if `pkg-config gtk+-2.0 --atleast-version=2.22`; then
  HAVE_GTK_2_22="yes"
  AC_DEFINE(HAVE_GTK_2_22, 1, [We have at least gtk version 2.22])
else
  HAVE_GTK_2_22="no"
fi;

if `pkg-config gtk+-2.0 --atleast-version=2.18`; then
  HAVE_GTK_2_18="yes"
  AC_DEFINE(HAVE_GTK_2_18, 1, [We have at least gtk version 2.18])
else
  HAVE_GTK_2_18="no"
fi;

if `pkg-config glib-2.0 --atleast-version=2.32`; then
  HAVE_GLIB_2_32="yes"
  AC_DEFINE(HAVE_GLIB_2_32, 1, [We have at least glib version 2.32])
else
  HAVE_GLIB_2_32="no"
fi;

if `pkg-config glib-2.0 --atleast-version=2.36`; then
  HAVE_GLIB_2_36="yes"
  AC_DEFINE(HAVE_GLIB_2_36, 1, [We have at least glib version 2.36])
else
  HAVE_GLIB_2_36="no"
fi;

dnl GTK can be disabled if required

AC_ARG_ENABLE(gtk, AC_HELP_STRING([--disable-gtk], [disable use of GTK]), enable_gtk=$enableval, enable_gtk="yes")

if test "$enable_gtk" = "yes"; then
	AC_DEFINE(USE_GTK, 1, [Use GTK])
else
	X11_LDFLAGS=`pkg-config --libs-only-l x11`
fi
AM_CONDITIONAL(ENABLE_GTK, test "$enable_gtk" = "yes")

dnl -opengl----------------------------------------------

AC_ARG_ENABLE(system-gtkglext, AC_HELP_STRING([--enable-system-gtkglext], [disable local opengl extension for gtk]), enable_system_gtkglext=$enableval, enable_system_gtkglext="no")

AC_ARG_ENABLE(
	[opengl],
	[  --disable-opengl        dont use the opengl canvas],
	[
		case "$enableval" in
			"yes")
				enable_opengl="yes"
				;;
			"no")
				AC_MSG_WARN([opengl preference specifically disabled])
				enable_opengl="no"
				;;
			*)
				AC_MSG_ERROR([must use --enable-dbus(=yes/no) or --disable-dbus])
				;;
		esac
	],
	[
		enable_opengl="yes"
	]
)
if test "x$enable_opengl" = "xyes" ; then
	if test "$enable_gtk" = "yes"; then
		OPENGL_CFLAGS=`pkg-config --cflags-only-I gl`
		OPENGL_LDFLAGS=`pkg-config --libs-only-l gl`

		AS_IF([test "$enable_system_gtkglext" = "yes"], [
			PKG_CHECK_MODULES(GTKGLEXT, [gtkglext-1.0], [OPENGL_CFLAGS=`pkg-config --cflags-only-I gtkglext-1.0`], [AC_MSG_ERROR([Gtkglext not found]) enable_system_gtkglext=no])
			])
	else
		OPENGL_CFLAGS=`pkg-config --cflags-only-I gl`
		OPENGL_LDFLAGS=`pkg-config --libs-only-l gl`
	fi
	AC_DEFINE(USE_OPENGL, 1, [Opengl features are enabled])
	AC_DEFINE(USE_FBO, 1, [enable support for opengl framebuffer objects for offscreen rendering])
	AC_DEFINE(USE_FX, 1, [enable a single additional pre-zoom shader fx pass])
	AC_DEFINE(WF_USE_TEXTURE_CACHE, 1, [cannot currently disable if opengl used])
fi
AM_CONDITIONAL(ENABLE_OPENGL, test $enable_opengl = "yes")
AM_CONDITIONAL(ENABLE_SYSTEM_GTKGLEXT, test "$enable_system_gtkglext" = "yes")
AC_SUBST(OPENGL_CFLAGS)
AC_SUBST(OPENGL_LDFLAGS)

dnl -----------------------------------------------------

AC_ARG_ENABLE(ffmpeg,   AC_HELP_STRING([--disable-ffmpeg], [disable use of FFMPEG]), enable_ffmpeg=$enableval, enable_ffmpeg="auto")

AS_IF([test "$enable_ffmpeg" != "no"], [
	PKG_CHECK_MODULES(FFMPEG, [libavformat >= 51.0.0 libavcodec >= 51.0.0 libavutil >= 50.0.0], enable_ffmpeg=yes, enable_ffmpeg=no)
	])

if test "$enable_ffmpeg" = "yes"; then
	AC_DEFINE(USE_FFMPEG,1,[Use ffmpeg audio decoder])
fi
AM_CONDITIONAL(ENABLE_FFMPEG, test "$enable_ffmpeg" = "yes")
FFMPEG_LDFLAGS=${FFMPEG_LIBS}
AC_SUBST(FFMPEG_CFLAGS)
AC_SUBST(FFMPEG_LDFLAGS)

dnl -----------------------------------------------------

AS_IF([test "$enable_sdl" != "no"], [
	PKG_CHECK_MODULES(SDL, sdl2, [AC_DEFINE(USE_SDL, 1, Enable support for showing waveforms in SDL applications) enable_sdl="yes"],[
	enable_sdl="no"])
])
AM_CONDITIONAL(ENABLE_SDL, test "$enable_sdl" = "yes")
AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_LDFLAGS)

dnl -----------------------------------------------------

AC_ARG_ENABLE(
	[lv2],
	[  --enable-lv2            build with lv2 support (default=no). Currently this option doesnt do anything useful.],
	[
		enable_lv2="$enableval"
		if test "$enableval" == "yes" && `pkg-config lilv-0`; then
			LV2_CFLAGS=`pkg-config --cflags-only-I lilv-0``pkg-config --cflags-only-I suil-0`
			LV2_LDFLAGS=`pkg-config --libs lilv-0``pkg-config --libs suil-0``pkg-config --libs jack`
			AC_DEFINE(USE_LV2, 1, [enable support for LV2 plugins])
		else 
			LV2_CFLAGS=
			enable_lv2="no"
		fi;
	],
	[
		enable_lv2="no"
	]
)
AM_CONDITIONAL(ENABLE_LV2, test "$enable_lv2" != "no")
AC_SUBST(LV2_LDFLAGS)
AC_SUBST(LV2_CFLAGS)

dnl -----------------------------------------------------

AS_IF([test "$enable_libass" != "no"], [
	PKG_CHECK_MODULES(LIBASS, libass, [AC_DEFINE(USE_LIBASS, 1, Use libass for text display) enable_libass="yes"],[
	enable_libass="no"])
])
AM_CONDITIONAL(ENABLE_LIBASS, test "$enable_libass" = "yes")
AC_SUBST(LIBASS_CFLAGS)
dnl the libass pc file seems to be missing one dir:
if test "$enable_libass" = "yes"; then
	LIBASS_CFLAGS=${LIBASS_CFLAGS}" -I/usr/include/ass"
	LIBASS_LDFLAGS=${LIBASS_LIBS}
	AC_SUBST(LIBASS_LDFLAGS)
fi

dnl AM_CONDITIONAL(ENABLE_GDK_PIXBUF, test "yes" != "no")
AC_DEFINE(USE_GDK_PIXBUF, 1, [enable support for Gdk Pixbuf])

dnl ------------------ test progs ------------------------

AC_ARG_ENABLE(test, AS_HELP_STRING([--enable-test],[ compile the test programs]), enable_test=yes, enable_test=no)

AM_CONDITIONAL(BUILD_TESTS, test "$enable_test" = "yes")
if test "$enable_test" = "yes"; then
	AC_DEFINE(USE_TEST, 1, [Add additional code to support test applications])
fi

dnl -----------------------------------------------------

# Local version of Gtkglext only works with X11
if test "$enable_system_gtkglext" != "no"; then
	AC_DEFINE(USE_SYSTEM_GTKGLEXT, 1, [do not use the internal Gtkglext library])
fi

dnl -----------------------------------------------------

AC_DEFINE(USE_FRAME_CLOCK, 1, [Synchronise drawing to screen refresh])

# extra CXXFLAGS that should always be used
dnl CXXFLAGS="$CXXFLAGS -pipe -fmessage-length=139 -fdiagnostics-show-location=every-line"
AM_CFLAGS="$CFLAGS -pipe -fmessage-length=139 -fdiagnostics-show-location=every-line"

AC_CONFIG_FILES([
	libwaveform.pc
	Makefile
	agl/Makefile
	agl/shaders/Makefile
	agl/behaviours/Makefile
	transition/Makefile
	sdl/Makefile
	gtkglext-1.0/Makefile
	decoder/Makefile
	waveform/Makefile
	waveform/actors/Makefile
	waveform/loaders/Makefile
	shaders/Makefile
	test/Makefile
	test/gen/Makefile
])
AC_OUTPUT

AC_MSG_RESULT([])
AC_MSG_RESULT([Libwaveform:])
AC_MSG_RESULT([Building opengl:          $enable_opengl])
AC_MSG_RESULT([Building gtk:             $enable_gtk])
AC_MSG_RESULT([Building sdl:             $enable_sdl])
AC_MSG_RESULT([Building libass:          $enable_libass])
AC_MSG_RESULT([Building ffmpeg:          $enable_ffmpeg])
AC_MSG_RESULT([Building tests:           $enable_test])
AC_MSG_RESULT([Use system GtkGL:         $enable_system_gtkglext])
dnl AC_MSG_RESULT([Building lv2:             $enable_lv2 (TODO)])
AC_MSG_RESULT([])

