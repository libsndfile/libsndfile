## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

SUBDIRS = include .

INCLUDES = -I$(top_srcdir)/src/FLAC/include -I$(top_srcdir)/src/FLAC/include/share \
	-I$(top_srcdir)/src/FLAC/src/share/replaygain_syn/include

noinst_LTLIBRARIES = libreplaygain_synthesis.la

libreplaygain_synthesis_la_SOURCES = replaygain_synthesis.c

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
