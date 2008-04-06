## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

INCLUDES = -I$(top_srcdir)/src/VORBIS/include -I$(top_srcdir)/src/OGG/include

noinst_PROGRAMS = test

test_SOURCES = util.c util.h write_read.c write_read.h test.c
test_LDADD = ../lib/libvorbisenc.la ../lib/libvorbis.la $(top_builddir)/src/OGG/libogg.la

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"

check: test$(EXEEXT)
	./test$(EXEEXT)
	
