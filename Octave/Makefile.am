## Process this file with automake to produce Makefile.in

# Prevent any extension.
EXEEXT =

EXTRA_DIST = sndfile_load.m sndfile_save.m sndfile_play.m

octconfigdir = $(exec_prefix)/share/octave/site/m
octconfig_DATA = sndfile_load.m sndfile_save.m sndfile_play.m

OCT_CXXFLAGS = $(shell $(MKOCTFILE) -p ALL_CXXFLAGS)

# OCT_LIB_DIR = $(shell $(MKOCTFILE) -p LFLAGS)
# OCT_LIBS = $(shell $(MKOCTFILE) -p OCTAVE_LIBS)

OCT_LIB_DIR = -L/usr/lib/octave-2.9.9
OCT_LIBS = -loctinterp -loctave -lcruft

SNDFILEDIR = ../src
INCLUDES = -I$(srcdir)/$(SNDFILEDIR)

bin_PROGRAMS = sfread.oct

noinst_HEADERS = format.h

sfread_oct_SOURCES = format.cc sfread.cc
sfread_oct_CXXFLAGS = $(OCT_CXXFLAGS) $(INCLUDES)
sfread_oct_LDADD = $(OCT_LIB_DIR) $(OCT_LIBS) $(SNDFILEDIR)/libsndfile.la


sfwrite_oct_SOURCES = format.cc sfwrite.cc
sfwrite_oct_CXXFLAGS = $(OCT_CXXFLAGS) $(INCLUDES)
sfwrite_oct_LDADD = $(OCT_LIB_DIR) $(OCT_LIBS) $(SNDFILEDIR)/libsndfile.la

