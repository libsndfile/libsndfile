## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sndfile-info sndfile-play sndfile-convert sndfile-jackplay sndfile-cmp \
				sndfile-metadata-set sndfile-metadata-get

noinst_PROGRAMS = make_sine sfprocess list_formats generate sndfilehandle

# This is the BeOS version of sndfile-play. It needs to be compiled with the C++
# compiler.
EXTRA_DIST = sndfile-play-beos.cpp test-sndfile-metadata-set.py

OS_SPECIFIC_CFLAGS = @OS_SPECIFIC_CFLAGS@
OS_SPECIFIC_LINKS = @OS_SPECIFIC_LINKS@

SNDFILEDIR = ../src
INCLUDES = -I$(srcdir)/$(SNDFILEDIR) $(OS_SPECIFIC_CFLAGS)

sndfile_info_SOURCES = sndfile-info.c
sndfile_info_LDADD = $(SNDFILEDIR)/libsndfile.la

sndfile_play_SOURCES = sndfile-play.c
sndfile_play_LDADD = $(SNDFILEDIR)/libsndfile.la $(OS_SPECIFIC_LINKS) $(ALSA_LIBS)

sndfile_jackplay_SOURCES = sndfile-jackplay.c
sndfile_jackplay_CFLAGS = $(JACK_CFLAGS)
sndfile_jackplay_LDADD = $(SNDFILEDIR)/libsndfile.la $(JACK_LIBS)

sndfile_convert_SOURCES = sndfile-convert.c common.c common.h
sndfile_convert_LDADD = $(SNDFILEDIR)/libsndfile.la

sndfile_cmp_SOURCES = sndfile-cmp.c
sndfile_cmp_LDADD = $(SNDFILEDIR)/libsndfile.la

sndfile_metadata_set_SOURCES = sndfile-metadata-set.c common.c common.h
sndfile_metadata_set_LDADD = $(SNDFILEDIR)/libsndfile.la

sndfile_metadata_get_SOURCES = sndfile-metadata-get.c
sndfile_metadata_get_LDADD = $(SNDFILEDIR)/libsndfile.la

make_sine_SOURCES = make_sine.c
make_sine_LDADD = $(SNDFILEDIR)/libsndfile.la

sfprocess_SOURCES = sfprocess.c
sfprocess_LDADD = $(SNDFILEDIR)/libsndfile.la

list_formats_SOURCES = list_formats.c
list_formats_LDADD = $(SNDFILEDIR)/libsndfile.la

generate_SOURCES = generate.c
generate_LDADD = $(SNDFILEDIR)/libsndfile.la

sndfilehandle_SOURCES = sndfilehandle.cc
sndfilehandle_LDADD = $(SNDFILEDIR)/libsndfile.la

