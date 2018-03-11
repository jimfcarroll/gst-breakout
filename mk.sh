#!/bin/sh -x

##################################################
# preamble
cd "$(dirname "$0")"
CURDIR="$(pwd -P)"
##################################################

rm -rf build
mkdir -p build

gcc -g -ggdb -fPIC -c `pkg-config --cflags gstreamer-video-1.0` -o $CURDIR/build/gstbreakout.o $CURDIR/src/gstbreakout.c

g++ -rdynamic -shared -o build/gstbreakout.so $CURDIR/build/gstbreakout.o `pkg-config --libs gstreamer-video-1.0`

