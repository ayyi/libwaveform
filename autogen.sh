#!/bin/sh

set -e

echo 'libwaveform: generating build files ...'

git submodule update --init --recursive

libtoolize --automake
aclocal
autoheader -Wall
automake --gnu --add-missing -Wall
autoconf
touch Makefile.in

(cd lib/agl/gtkglext-1.0 &&
	./autogen.sh)
cd lib/agl/lib/gtkglext3 &&
	./autogen
