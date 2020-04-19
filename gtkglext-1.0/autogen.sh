#!/bin/sh

echo 'Generating necessary files...'
libtoolize --automake
aclocal
autoheader -Wall
automake --gnu --add-missing -Wall

# fix dist path issues in ac_config subdirectory
sed -i -e 's/..\/compile/compile/' Makefile.in
sed -i -e 's/..\/config.guess/config.guess/' Makefile.in
sed -i -e 's/..\/config.sub/config.sub/' Makefile.in
sed -i -e 's/..\/install-sh/install-sh/' Makefile.in
sed -i -e 's/..\/ltmain.sh/ltmain.sh/' Makefile.in
sed -i -e 's/..\/missing/missing/' Makefile.in

autoconf
touch Makefile.in

