## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sndfile-regtest

noinst_HEADERS = regtest.h

SNDFILEDIR =../src
INCLUDES = -I$(srcdir)/$(SNDFILEDIR) $(SQLITE3_CFLAGS) $(OS_SPECIFIC_CFLAGS)

sndfile_regtest_SOURCES = sndfile-regtest.c database.c checksum.c
sndfile_regtest_LDADD = $(SNDFILEDIR)/libsndfile.la $(SQLITE3_LIBS)

