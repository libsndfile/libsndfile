## Process this file with automake to produce Makefile.in

SUBDIRS = include .

INCLUDES = -I$(top_srcdir)/src/FLAC/share/replaygain_synthesis/include \
	-I$(top_srcdir)/src/FLAC -I$(top_srcdir)/src/FLAC/include \
	-I$(top_srcdir)/src/FLAC/include/share
	

noinst_LTLIBRARIES = libreplaygain_synthesis.la

libreplaygain_synthesis_la_SOURCES = replaygain_synthesis.c

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
