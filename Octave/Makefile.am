## Process this file with automake to produce Makefile.in

EXTRA_DIST = sndfile_load.m sndfile_save.m sndfile_play.m

octconfigdir = $(exec_prefix)/share/octave/site/m
octconfig_DATA = sndfile_load.m sndfile_save.m sndfile_play.m
	


%.o: %.c ; $(MKOCTFILE) -c $<
%.o: %.f ; $(MKOCTFILE) -c $<
%.o: %.cc ; $(MKOCTFILE) -c $<
%.oct: %.cc ; $(MKOCTFILE) $<
