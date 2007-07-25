## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

INCLUDES = -I$(top_srcdir)/src/FLAC/include/share

noinst_LTLIBRARIES = libutf8.la

noinst_HEADERS = iconvert.h

libutf8_la_SOURCES = charset.c charset.h iconvert.c utf8.c

EXTRA_DIST = \
	charmaps.h \
	makemap.c \
	charset_test.c \
	charsetmap.h

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
