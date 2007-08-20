## Process this file with automake to produce Makefile.in

SUBDIRS = OGG VORBIS FLAC GSM610 G72x

INCLUDES = -I$(top_srcdir)/src/OGG/include

lib_LTLIBRARIES = libsndfile.la
include_HEADERS = sndfile.hh
nodist_include_HEADERS = sndfile.h

noinst_LTLIBRARIES = libcommon.la

OS_SPECIFIC_CFLAGS = @OS_SPECIFIC_CFLAGS@
OS_SPECIFIC_LINKS = @OS_SPECIFIC_LINKS@

EXTRA_DIST = sndfile.h.in config.h.in test_endswap.tpl test_endswap.def \
			Symbols.linux Symbols.darwin libsndfile.def cygsndfile.def \
			create_symbols_file.py binheader_writef_check.py

noinst_HEADERS = common.h sfconfig.h sfendian.h float_cast.h wav_w64.h sf_unistd.h

noinst_PROGRAMS = test_endswap test_file_io test_conversions test_log_printf \
		test_audio_detect

COMMON = common.c file_io.c command.c pcm.c ulaw.c alaw.c float32.c \
		double64.c ima_adpcm.c ms_adpcm.c gsm610.c dwvw.c vox_adpcm.c \
		interleave.c strings.c dither.c broadcast.c audio_detect.c

FILESPECIFIC = sndfile.c aiff.c au.c avr.c caf.c dwd.c flac.c g72x.c htk.c ircam.c \
		macbinary3.c macos.c mat4.c mat5.c nist.c ogg.c paf.c pvf.c raw.c rx2.c sd2.c \
		sds.c svx.c txw.c voc.c wve.c w64.c wav_w64.c wav.c xi.c

# MinGW requires -no-undefined if a DLL is to be built.
libsndfile_la_LDFLAGS = -no-undefined -version-info @SHARED_VERSION_INFO@ @SHLIB_VERSION_ARG@ @OS_SPECIFIC_LIBLINKS@
libsndfile_la_SOURCES = $(FILESPECIFIC) $(noinst_HEADERS)
nodist_libsndfile_la_SOURCES = $(nodist_include_HEADERS)
libsndfile_la_LIBADD = libcommon.la GSM610/libgsm.la G72x/libg72x.la OGG/libogg.la \
		VORBIS/lib/libvorbis.la VORBIS/lib/libvorbisenc.la FLAC/src/libFLAC/libFLAC.la -lm

libcommon_la_SOURCES = $(COMMON)

test_endswap_SOURCES = test_endswap.c

test_file_io_CFLAGS = $(AM_CFLAGS)
test_file_io_SOURCES = test_file_io.c
test_file_io_LDADD = libcommon.la

test_log_printf_CFLAGS = $(AM_CFLAGS)
test_log_printf_SOURCES = test_log_printf.c
test_log_printf_LDADD = libcommon.la

test_conversions_CFLAGS = $(AM_CFLAGS)
test_conversions_SOURCES = test_conversions.c
test_conversions_LDADD = libcommon.la

test_audio_detect_CFLAGS = $(AM_CFLAGS)
test_audio_detect_SOURCES = test_audio_detect.c
test_audio_detect_LDADD = libcommon.la

test_endswap.c: test_endswap.def test_endswap.tpl
	autogen --writable test_endswap.def

genfiles : test_endswap.c Symbols.linux Symbols.darwin libsndfile.def cygsndfile.def

# Two test programs.
# It is not possible to place these in the tests/ directory because they
# need access to the internals of the SF_PRIVATE struct.

check: test_endswap test_file_io test_conversions test_log_printf
	@echo
	@echo
	@echo
	@echo "============================================================"
	@if [ -x /usr/bin/python2.4 ]; then $(srcdir)/binheader_writef_check.py $(srcdir)/*.c ; fi
	./test_endswap
	./test_file_io
	./test_conversions
	./test_log_printf
	./test_audio_detect
	@echo "============================================================"
	@echo
	@echo
	@echo

#======================================================================
# Generate an OS specific Symbols files. This is done when the author
# builds the distribution tarball. There should be not need for the
# end user to create these files.

Symbols.linux: create_symbols_file.py
	./create_symbols_file.py linux $(VERSION) > Symbols.linux

Symbols.darwin: create_symbols_file.py
	./create_symbols_file.py darwin $(VERSION) > Symbols.darwin

libsndfile.def: create_symbols_file.py
	./create_symbols_file.py win32 $(VERSION) > libsndfile.def

cygsndfile.def: create_symbols_file.py
	./create_symbols_file.py cygwin $(VERSION) > cygsndfile.def

# Fake dependancy to force the creation of these files.
sndfile.o : Symbols.linux Symbols.darwin libsndfile.def cygsndfile.def

#======================================================================
# Disable autoheader.
AUTOHEADER=echo

# Dependancies.

aiff.c au.c g72x.c ircam.c mat4.c mat5.c nist.c paf.c pvf.c : sndfile.h common.h
raw.c svx.c voc.c w64.c wav.c wav_w64.c htk.c sd2.c rx2.c txw.c : sndfile.h common.h
sds.c wve.c dwd.c ogg.c xi.c sndfile.c common.c file_io.c : sndfile.h common.h
command.c pcm.c ulaw.c alaw.c float32.c double64.c ima_adpcm.c : sndfile.h common.h
ms_adpcm.c gsm610.c dwvw.c vox_adpcm.c interleave.c strings.c : sndfile.h common.h
dither.c : sndfile.h common.h

## Do not edit or modify anything in this comment block.
## The arch-tag line is a file identity tag for the GNU Arch
## revision control system.
##
## arch-tag: fc3511e6-4230-4bcb-9c86-f728d7a06fe7

