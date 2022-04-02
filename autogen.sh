#!/bin/sh

echo 'libwaveform: generating build files ...'
libtoolize --automake
aclocal
autoheader -Wall
automake --gnu --add-missing -Wall
autoconf
touch Makefile.in
