## Process this file with automake to produce Makefile.in

noinst_PROGRAMS = sfversion floating_point_test write_read_test \
	lossy_comp_test error_test ulaw_test alaw_test dwvw_test \
	peak_chunk_test command_test stdin_test stdout_test stdio_test \
	pcm_test headerless_test pipe_test benchmark misc_test raw_test \
	string_test open_fail_test multi_file_test dither_test \
	scale_clip_test win32_test fix_this aiff_rw_test

SNDFILEDIR = ../src
INCLUDES = -I$(srcdir)/$(SNDFILEDIR)
noinst_HEADERS = dft_cmp.h utils.h

autogen_sources = write_read_test.tpl write_read_test.def	\
					pcm_test.tpl pcm_test.def				\
					utils.tpl utils.def						\
					command_test.tpl command_test.def		\
					scale_clip_test.tpl scale_clip_test.def	\
					benchmark.tpl benchmark.def

EXTRA_DIST = $(autogen_sources)

#===============================================================================

sfversion_SOURCES = sfversion.c
sfversion_LDADD = $(SNDFILEDIR)/libsndfile.la

write_read_test_SOURCES = utils.c write_read_test.c
write_read_test_LDADD = $(SNDFILEDIR)/libsndfile.la

lossy_comp_test_SOURCES = utils.c lossy_comp_test.c
lossy_comp_test_LDADD = $(SNDFILEDIR)/libsndfile.la

fix_this_SOURCES = utils.c fix_this.c
fix_this_LDADD = $(SNDFILEDIR)/libsndfile.la

error_test_SOURCES = error_test.c
error_test_LDADD = $(SNDFILEDIR)/libsndfile.la

ulaw_test_SOURCES = utils.c ulaw_test.c
ulaw_test_LDADD = $(SNDFILEDIR)/libsndfile.la

alaw_test_SOURCES = utils.c alaw_test.c
alaw_test_LDADD = $(SNDFILEDIR)/libsndfile.la

aiff_rw_test_SOURCES = utils.c aiff_rw_test.c
aiff_rw_test_LDADD = $(SNDFILEDIR)/libsndfile.la

command_test_SOURCES = command_test.c utils.c
command_test_LDADD = $(SNDFILEDIR)/libsndfile.la

pcm_test_SOURCES = pcm_test.c utils.c
pcm_test_LDADD = $(SNDFILEDIR)/libsndfile.la

headerless_test_SOURCES = utils.c headerless_test.c
headerless_test_LDADD = $(SNDFILEDIR)/libsndfile.la

stdin_test_SOURCES = stdin_test.c utils.c
stdin_test_LDADD = $(SNDFILEDIR)/libsndfile.la

stdout_test_SOURCES = stdout_test.c
stdout_test_LDADD = $(SNDFILEDIR)/libsndfile.la

stdio_test_SOURCES = stdio_test.c utils.c
stdio_test_LDADD = $(SNDFILEDIR)/libsndfile.la

pipe_test_SOURCES = pipe_test.c utils.c
pipe_test_LDADD = $(SNDFILEDIR)/libsndfile.la

benchmark_SOURCES = benchmark.c
benchmark_LDADD = $(SNDFILEDIR)/libsndfile.la

misc_test_SOURCES = misc_test.c utils.c
misc_test_LDADD = $(SNDFILEDIR)/libsndfile.la

raw_test_SOURCES = raw_test.c utils.c
raw_test_LDADD = $(SNDFILEDIR)/libsndfile.la

string_test_SOURCES = string_test.c utils.c
string_test_LDADD = $(SNDFILEDIR)/libsndfile.la

dither_test_SOURCES = dither_test.c utils.c
dither_test_LDADD = $(SNDFILEDIR)/libsndfile.la

open_fail_test_SOURCES = open_fail_test.c utils.c
open_fail_test_LDADD = $(SNDFILEDIR)/libsndfile.la

multi_file_test_SOURCES = multi_file_test.c utils.c
multi_file_test_LDADD = $(SNDFILEDIR)/libsndfile.la

win32_test_SOURCES = win32_test.c
win32_test_LDADD = 

# Lite remove start
dwvw_test_SOURCES = utils.c dwvw_test.c
dwvw_test_LDADD = $(SNDFILEDIR)/libsndfile.la

floating_point_test_SOURCES = utils.c dft_cmp.c floating_point_test.c
floating_point_test_LDADD = $(SNDFILEDIR)/libsndfile.la

peak_chunk_test_SOURCES = peak_chunk_test.c utils.c
peak_chunk_test_LDADD = $(SNDFILEDIR)/libsndfile.la

scale_clip_test_SOURCES = scale_clip_test.c utils.c
scale_clip_test_LDADD = $(SNDFILEDIR)/libsndfile.la
# Lite remove end

#===============================================================================

write_read_test.c: write_read_test.def write_read_test.tpl
	autogen --writable --source-time write_read_test.def

pcm_test.c: pcm_test.def pcm_test.tpl
	autogen --writable --source-time pcm_test.def

utils.c: utils.def utils.tpl
	autogen --writable --source-time utils.def

command_test.c: command_test.def command_test.tpl
	autogen --writable --source-time command_test.def

scale_clip_test.c: scale_clip_test.def scale_clip_test.tpl
	autogen --writable --source-time scale_clip_test.def

benchmark.c: benchmark.def benchmark.tpl
	autogen --writable --source-time benchmark.def

#===============================================================================

check: generic-tests wav-tests aiff-tests au-tests raw-tests paf-tests \
	svx-tests nist-tests ircam-tests voc-tests w64-tests mat4-tests \
	mat5-tests pvf-tests xi-tests htk-tests avr-tests sds-tests \
	stdio-tests

generic-tests : error_test$(EXEEXT) ulaw_test$(EXEEXT) alaw_test$(EXEEXT) command_test$(EXEEXT) floating_point_test$(EXEEXT) pcm_test$(EXEEXT)
	uname -a
	./error_test$(EXEEXT)
	./pcm_test$(EXEEXT)
	./ulaw_test$(EXEEXT)
	./alaw_test$(EXEEXT)
	./dwvw_test$(EXEEXT)
	./command_test$(EXEEXT) ver
	./command_test$(EXEEXT) norm
	./command_test$(EXEEXT) format
	./command_test$(EXEEXT) peak
	./command_test$(EXEEXT) trunc
	./floating_point_test$(EXEEXT)
	./scale_clip_test$(EXEEXT)
	./headerless_test$(EXEEXT)
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed common tests."
	@echo "----------------------------------------------------------------------"

wav-tests: write_read_test$(EXEEXT) lossy_comp_test$(EXEEXT) peak_chunk_test$(EXEEXT) misc_test$(EXEEXT) multi_file_test$(EXEEXT)
	./write_read_test$(EXEEXT) wav
	./lossy_comp_test$(EXEEXT) wav_ima
	./lossy_comp_test$(EXEEXT) wav_msadpcm
	./lossy_comp_test$(EXEEXT) wav_ulaw
	./lossy_comp_test$(EXEEXT) wav_alaw
	./lossy_comp_test$(EXEEXT) wav_gsm610
	./peak_chunk_test$(EXEEXT) wav
	./misc_test$(EXEEXT) wav
	./string_test$(EXEEXT) wav
	./multi_file_test$(EXEEXT) wav
	./open_fail_test$(EXEEXT) wav
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on WAV files."
	@echo "----------------------------------------------------------------------"

aiff-tests: write_read_test$(EXEEXT) lossy_comp_test$(EXEEXT) peak_chunk_test$(EXEEXT) misc_test$(EXEEXT) aiff_rw_test$(EXEEXT)
	./write_read_test$(EXEEXT) aiff
	./lossy_comp_test$(EXEEXT) aiff_ulaw
	./lossy_comp_test$(EXEEXT) aiff_alaw
	./lossy_comp_test$(EXEEXT) aiff_gsm610
	@echo "=========================="
	@echo "./lossy_comp_test$(EXEEXT) aiff_ima"
	@echo "=========================="
	./peak_chunk_test$(EXEEXT) aiff
	./misc_test$(EXEEXT) aiff
	./string_test$(EXEEXT) aiff
	./multi_file_test$(EXEEXT) aiff
	./aiff_rw_test$(EXEEXT)
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on AIFF files."
	@echo "----------------------------------------------------------------------"

au-tests: write_read_test$(EXEEXT) lossy_comp_test$(EXEEXT) misc_test$(EXEEXT)
	./write_read_test$(EXEEXT) au
	./lossy_comp_test$(EXEEXT) au_ulaw
	./lossy_comp_test$(EXEEXT) au_alaw
	./lossy_comp_test$(EXEEXT) au_g721
	./lossy_comp_test$(EXEEXT) au_g723
	./misc_test$(EXEEXT) au
	./multi_file_test$(EXEEXT) wav
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on AU files."
	@echo "----------------------------------------------------------------------"

raw-tests: write_read_test$(EXEEXT) lossy_comp_test$(EXEEXT) raw_test$(EXEEXT)
	./write_read_test$(EXEEXT) raw
	./lossy_comp_test$(EXEEXT) raw_ulaw
	./lossy_comp_test$(EXEEXT) raw_alaw
	./lossy_comp_test$(EXEEXT) vox_adpcm
	./raw_test
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on RAW (header-less) files."
	@echo "----------------------------------------------------------------------"

w64-tests: write_read_test$(EXEEXT) lossy_comp_test$(EXEEXT) misc_test$(EXEEXT)
	./write_read_test$(EXEEXT) w64
	./lossy_comp_test$(EXEEXT) w64_ima
	./lossy_comp_test$(EXEEXT) w64_msadpcm
	./lossy_comp_test$(EXEEXT) w64_ulaw
	./lossy_comp_test$(EXEEXT) w64_alaw
	./lossy_comp_test$(EXEEXT) w64_gsm610
	./misc_test$(EXEEXT) w64
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on W64 files."
	@echo "----------------------------------------------------------------------"

# Lite remove start
paf-tests: write_read_test$(EXEEXT) misc_test$(EXEEXT)
	./write_read_test$(EXEEXT) paf
	./misc_test$(EXEEXT) paf
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on PAF files."
	@echo "----------------------------------------------------------------------"

svx-tests: write_read_test$(EXEEXT) misc_test$(EXEEXT)
	./write_read_test$(EXEEXT) svx
	./misc_test$(EXEEXT) svx
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on SVX files."
	@echo "----------------------------------------------------------------------"

nist-tests: write_read_test$(EXEEXT) lossy_comp_test$(EXEEXT) misc_test$(EXEEXT)
	./write_read_test$(EXEEXT) nist
	./lossy_comp_test$(EXEEXT) nist_ulaw
	./lossy_comp_test$(EXEEXT) nist_alaw
	./misc_test$(EXEEXT) nist
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on NIST files."
	@echo "----------------------------------------------------------------------"

ircam-tests: write_read_test$(EXEEXT) lossy_comp_test$(EXEEXT) misc_test$(EXEEXT)
	./write_read_test$(EXEEXT) ircam
	./lossy_comp_test$(EXEEXT) ircam_ulaw
	./lossy_comp_test$(EXEEXT) ircam_alaw
	./misc_test$(EXEEXT) ircam
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on IRCAM files."
	@echo "----------------------------------------------------------------------"

voc-tests: write_read_test$(EXEEXT) lossy_comp_test$(EXEEXT) misc_test$(EXEEXT)
	./write_read_test$(EXEEXT) voc
	./lossy_comp_test$(EXEEXT) voc_ulaw
	./lossy_comp_test$(EXEEXT) voc_alaw
	./misc_test$(EXEEXT) voc
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on VOC files."
	@echo "----------------------------------------------------------------------"

mat4-tests: write_read_test$(EXEEXT) misc_test$(EXEEXT)
	./write_read_test$(EXEEXT) mat4
	./misc_test$(EXEEXT) mat4
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on MAT4 files."
	@echo "----------------------------------------------------------------------"

mat5-tests: write_read_test$(EXEEXT) misc_test$(EXEEXT)
	./write_read_test$(EXEEXT) mat5
	./misc_test$(EXEEXT) mat5
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on MAT5 files."
	@echo "----------------------------------------------------------------------"

pvf-tests: write_read_test$(EXEEXT) misc_test$(EXEEXT)
	./write_read_test$(EXEEXT) pvf
	./misc_test$(EXEEXT) pvf
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on PVF files."
	@echo "----------------------------------------------------------------------"

xi-tests: lossy_comp_test$(EXEEXT)
	./lossy_comp_test$(EXEEXT) xi_dpcm
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on XI files."
	@echo "----------------------------------------------------------------------"

htk-tests: write_read_test$(EXEEXT) misc_test$(EXEEXT)
	./write_read_test$(EXEEXT) htk
	./misc_test$(EXEEXT) htk
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on HTK files."
	@echo "----------------------------------------------------------------------"

avr-tests: write_read_test$(EXEEXT) misc_test$(EXEEXT)
	./write_read_test$(EXEEXT) avr
	./misc_test$(EXEEXT) avr
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on AVR files."
	@echo "----------------------------------------------------------------------"

sds-tests: write_read_test$(EXEEXT) misc_test$(EXEEXT)
	./write_read_test$(EXEEXT) sds
	./misc_test$(EXEEXT) sds
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed tests on SDS files."
	@echo "----------------------------------------------------------------------"

# Lite remove end

stdio-tests: stdio_test$(EXEEXT) stdin_test$(EXEEXT) stdout_test$(EXEEXT) pipe_test$(EXEEXT)
	./stdio_test$(EXEEXT)
	./pipe_test$(EXEEXT)
	@echo "----------------------------------------------------------------------"
	@echo "  `./sfversion$(EXEEXT)` passed stdio and pipe tests."
	@echo "----------------------------------------------------------------------"



## Do not edit or modify anything in this comment block.
## The arch-tag line is a file identity tag for the GNU Arch 
## revision control system.
##
## arch-tag: 820ec297-29b9-48a4-998f-48b17053f435

