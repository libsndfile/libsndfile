## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/src/FLAC/include -I$(top_srcdir)/src/FLAC

noinst_LTLIBRARIES = libgrabbag.la

libgrabbag_la_SOURCES = \
	cuesheet.c \
	file.c \
	picture.c \
	replaygain.c \
	seektable.c

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
