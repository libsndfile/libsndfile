#!/usr/bin/make -f

# Build libsndfile as a dynamic/shared library, but statically link to
# libFLAC, libogg and libvorbis

ogg_version = libogg-1.3.2
ogg_md5sum = 5c3a34309d8b98640827e5d0991a4015

vorbis_version = libvorbis-1.3.4
vorbis_md5sum = 55f2288055e44754275a17c9a2497391

flac_version = flac-1.3.0
flac_md5sum = 13b5c214cee8373464d3d65dee362cdd

#-------------------------------------------------------------------------------
# Code follows.

ogg_tarball = $(ogg_version).tar.xz
vorbis_tarball = $(vorbis_version).tar.xz
flac_tarball = $(flac_version).tar.xz

download_url = http://downloads.xiph.org/releases/
tarball_dir = Build/Tarballs
stamp_dir = Build/Stamp

build_dir = $(shell pwd)/Build
config_options = --prefix=$(build_dir) --disable-shared

help :
	@echo
	@echo "This script will build libsndfile as a dynamic/shared library but statically linked"
	@echo "to libFLAC, libogg and libvorbis. It should work on Linux and Mac OS X. It might"
	@echo "work on Windows with a correctly set up MinGW."
	@echo
	@echo "It requires all the normal build tools require to build libsndfile plus wget."
	@echo

clean :
	rm -rf Build/flac-* Build/libogg-* Build/libvorbis-*
	rm -rf Build/bin Build/include Build/lib Build/share
	rm -f Build/Stamp/install Build/Stamp/extract Build/Stamp/md5sum

Build/Stamp/tarballs :
	mkdir -p $(stamp_dir) $(tarball_dir)
	(cd $(tarball_dir) && wget $(download_url)ogg/$(ogg_tarball))
	(cd $(tarball_dir) && wget $(download_url)vorbis/$(vorbis_tarball))
	(cd $(tarball_dir) && wget $(download_url)flac/$(flac_tarball))
	touch $@

Build/Stamp/md5sum : Build/Stamp/tarballs
	test `md5sum $(tarball_dir)/$(ogg_tarball) | sed "s/ .*//"` = $(ogg_md5sum)
	test `md5sum $(tarball_dir)/$(vorbis_tarball) | sed "s/ .*//"` = $(vorbis_md5sum)
	test `md5sum $(tarball_dir)/$(flac_tarball) | sed "s/ .*//"` = $(flac_md5sum)
	touch $@

Build/Stamp/extract : Build/Stamp/md5sum
	(cd Build && tar xf Tarballs/$(ogg_tarball))
	(cd Build && tar xf Tarballs/$(flac_tarball))
	(cd Build && tar xf Tarballs/$(vorbis_tarball))
	touch $@

Build/Stamp/install : Build/Stamp/extract
	(cd Build/$(ogg_version) && CFLAGS=-fPIC ./configure $(config_options) && make all install)
	(cd Build/$(vorbis_version) && CFLAGS=-fPIC ./configure $(config_options) && make all install)
	(cd Build/$(flac_version) && CFLAGS=-fPIC ./configure $(config_options) && make all install)
	touch $@

Build/Stamp/build : Build/Stamp/install
	PKG_CONFIG_LIBDIR=Build/lib/pkgconfig ./configure
	sed -i 's#^EXTERNAL_CFLAGS.*#EXTERNAL_CFLAGS = -I/home/erikd/Git/libsndfile/Build/include#' src/Makefile
	sed -i 's#^EXTERNAL_LIBS.*#EXTERNAL_LIBS = -static /home/erikd/Git/libsndfile/Build/lib/libFLAC.la /home/erikd/Git/libsndfile/Build/lib/libogg.la /home/erikd/Git/libsndfile/Build/lib/libvorbis.la /home/erikd/Git/libsndfile/Build/lib/libvorbisenc.la -dynamic #' src/Makefile
	make
	make check

