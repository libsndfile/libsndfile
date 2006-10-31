## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/src/FLAC/include/share -I$(top_srcdir)/src/FLAC/include

noinst_LIBRARIES = libgetopt.a

libgetopt_a_SOURCES = getopt.c getopt1.c

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
