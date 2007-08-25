## Process this file with automake to produce Makefile.in

# Prevent any extension.
EXEEXT =

EXTRA_DIST = sndfile_load.m sndfile_save.m sndfile_play.m

octconfigdir = $(exec_prefix)/share/octave/site/m
octconfig_DATA = sndfile_load.m sndfile_save.m sndfile_play.m

OCTAVE_DEST_MDIR = @OCTAVE_DEST_MDIR@
OCTAVE_DEST_ODIR = @OCTAVE_DEST_ODIR@

OCT_CXXFLAGS = @OCT_CXXFLAGS@
OCT_LIB_DIR = @OCT_LIB_DIR@
OCT_LIBS = @OCT_LIBS@

SNDFILEDIR = ../src
INCLUDES = -I$(srcdir)/$(SNDFILEDIR)

oct_modules = sfread.oct sfwrite.oct

# Make these noinst so they can be installed manually.
noinst_PROGRAMS = $(oct_modules)

noinst_HEADERS = format.h

sfread_oct_SOURCES = format.cc sfread.cc
sfread_oct_CXXFLAGS = $(OCT_CXXFLAGS) $(INCLUDES)
sfread_oct_LDADD = $(OCT_LIB_DIR) $(OCT_LIBS) $(SNDFILEDIR)/libsndfile.la

sfwrite_oct_SOURCES = format.cc sfwrite.cc
sfwrite_oct_CXXFLAGS = $(OCT_CXXFLAGS) $(INCLUDES)
sfwrite_oct_LDADD = $(OCT_LIB_DIR) $(OCT_LIBS) $(SNDFILEDIR)/libsndfile.la

install-exec-local : $(oct_modules)
	@$(NORMAL_INSTALL)
	test -z "$(OCTAVE_DEST_ODIR)" || $(mkdir_p) "$(DESTDIR)$(OCTAVE_DEST_ODIR)"
	@list='$(oct_modules)'; for p in $$list; do \
	  p1=`echo $$p|sed 's/$(EXEEXT)$$//'`; \
	  if test -f $$p \
	     || test -f $$p1 \
	  ; then \
	    f=`echo "$$p1" | sed 's,^.*/,,;$(transform);s/$$/$(EXEEXT)/'`; \
	   echo " $(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL) '$$p' '$(DESTDIR)$(OCTAVE_DEST_ODIR)/$$f'"; \
	   $(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL) "$$p" "$(DESTDIR)$(OCTAVE_DEST_ODIR)/$$f" || exit 1; \
	  else :; fi; \
	done

uninstall-local :
	@$(NORMAL_UNINSTALL)
	@list='$(oct_modules)'; for p in $$list; do \
	  f=`echo "$$p" | sed 's,^.*/,,;s/$(EXEEXT)$$//;$(transform);s/$$/$(EXEEXT)/'`; \
	  echo " rm -f '$(DESTDIR)$(OCTAVE_DEST_ODIR)/$$f'"; \
	  rm -f "$(DESTDIR)$(OCTAVE_DEST_ODIR)/$$f"; \
	done
