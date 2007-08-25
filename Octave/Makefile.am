## Process this file with automake to produce Makefile.in

INCLUDES = -I../src

EXTRA_DIST = sndfile_load.m sndfile_save.m sndfile_play.m

octconfigdir = $(exec_prefix)/share/octave/site/m
octconfig_DATA = sndfile_load.m sndfile_save.m sndfile_play.m

all : sfread.oct sfwrite.oct

sfread.oct : format.o sfread.cc
	$(MKOCTFILE) $+ -lsndfile -o $@

sfwrite.oct : format.o sfwrite.cc
	$(MKOCTFILE) $(INCLUDES) $+ -lsndfile -o $@

format.o : format.cc
	$(MKOCTFILE) $(INCLUDES) -c $<

%.o: %.c ; $(MKOCTFILE) -c $<
%.o: %.f ; $(MKOCTFILE) -c $<
%.o: %.cc ; $(MKOCTFILE) -c $<
%.oct: %.cc ; $(MKOCTFILE) $<
