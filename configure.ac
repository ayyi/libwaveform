AC_PREREQ(2.59)
AC_INIT([Libwaveform],[0.1.0],[tim@orford.org])
AC_CONFIG_SRCDIR([waveform/peak.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

# LT_REVISION=1

AC_PROG_CXX
AC_C_CONST
AC_C_INLINE
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Library building stuff
AC_PROG_RANLIB
AC_PROG_LIBTOOL

privdatadir='${datadir}'/ayyi_meter
AC_SUBST(privdatadir)

# Check for debugging flag
debug="yes"
AC_ARG_ENABLE(debug,
	[AS_HELP_STRING(--enable-debug, [Enable debugging (false)])],
	[debug="$enableval"])
if test "$debug" = "yes"; then
  CFLAGS="-O0 -g -DDEBUG -Wall"
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
  CFLAGS="$CFLAGS -ansi -Wall -Wextra -Wno-unused-parameter -Wconversion -Winit-self"
  CXXFLAGS="$CXXFLAGS -ansi -Wall -Wextra -Wno-unused-parameter -Wconversion -Winit-self -Woverloaded-virtual -Wsign-promo"
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
FTGL_CFLAGS=`pkg-config --cflags ftgl`
AC_SUBST(FTGL_CFLAGS)
FTGL_LIBS=`pkg-config --libs ftgl`
AC_SUBST(FTGL_LIBS)

dnl -----------------------------------------------------

GTK_CFLAGS=`pkg-config gtk+-2.0 --cflags`
GTK_LDFLAGS=`pkg-config gtk+-2.0 --libs`
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LDFLAGS)

if `pkg-config gtk+-2.0 --atleast-version=2.12`; then
  HAVE_GTK_2_12="yes"
  AC_DEFINE(HAVE_GTK_2_12, 1, "we have at least gtk version 2.12")
else
  HAVE_GTK_2_12="no"
fi;

if `pkg-config gtk+-2.0 --atleast-version=2.10`; then
  HAVE_GTK_2_10="yes"
  AC_DEFINE(HAVE_GTK_2_10, 1, "we have at least gtk version 2.10")
else
  HAVE_GTK_2_10="no"
fi;

dnl -opengl----------------------------------------------

AM_CONDITIONAL(ENABLE_OPENGL, test "yes" = "yes")
enable_opengl="yes"
OPENGL_CFLAGS=`pkg-config --cflags-only-I gtkglext-1.0`
OPENGL_LDFLAGS=`pkg-config --libs-only-l gtkglext-1.0`
AC_SUBST(OPENGL_CFLAGS)
AC_SUBST(OPENGL_LDFLAGS)
AC_DEFINE(USE_OPENGL, 1, Opengl features are enabled)

dnl -----------------------------------------------------

GLU_LIBS=`pkg-config --libs-only-l glu`
dnl GLUT_LIBS="-lglut -lGLEW"
GLUT_LIBS=""
AC_SUBST(GLU_LIBS)
AC_SUBST(GLUT_LIBS)

dnl ------------------ test progs ------------------------

AC_ARG_ENABLE(test, AS_HELP_STRING([--enable-test],[ compile the test programs]), enable_test=yes, enable_test=no)

AM_CONDITIONAL(BUILD_TESTS, test "$enable_test" = "yes")

dnl -----------------------------------------------------

# extra CXXFLAGS that should always be used
dnl CXXFLAGS="$CXXFLAGS -pipe -fmessage-length=139 -fdiagnostics-show-location=every-line"
AM_CFLAGS="$CFLAGS -pipe  -fmessage-length=139 -fdiagnostics-show-location=every-line"

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([waveform/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_OUTPUT

AC_MSG_RESULT([])
AC_MSG_RESULT([Libwaveform:])
AC_MSG_RESULT([Building tests:           $enable_test])
AC_MSG_RESULT([])

