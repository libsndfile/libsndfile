## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/src/FLAC/include/share

noinst_LTLIBRARIES = libreplaygain_analysis.la

libreplaygain_analysis_la_SOURCES = replaygain_analysis.c

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
