#!/bin/sh
libtoolize --copy --force
aclocal
autoheader
automake --add-missing --copy
autoconf
if test x$1 != x--no-conf; then
    ./configure $@ || exit 1
fi