#!/usr/bin/make -f

# The auto tools MUST be run in the following order:
#
#	1.  aclocal
#	2.  libtoolize (if you use libtool)
#	3.  autoconf
#	4.  autoheader (if you use autoheader)
#	5.  automake (if you use automake)
#
# The following makefile runs these in the correct order according to their
# dependancies. It also makes up for Mac OSX's fucked-upped-ness.

ACLOCAL = aclocal
ACLOCAL_INC = -I M4

ifneq ($(shell uname -s), Darwin)
  LIBTOOLIZE = libtoolize
else
  # Fuck Apple! Why the hell did they rename libtoolize????
  LIBTOOLIZE = glibtoolize
  # Fink (and DarwinPorts/MacPorts) sucks as well, but this seems necessary.
  ACLOCAL_INC = $(ACLOCAL_INC) -I /opt/local/share/aclocal
endif

genfiles : config.status
	(cd src && make genfiles)
	(cd tests && make genfiles)

config.status: configure src/config.h.in Makefile.in src/Makefile.in tests/Makefile.in
	./configure --enable-gcc-werror

configure: ltmain.sh
	autoconf

Makefile.in: Makefile.am
	automake --copy --add-missing

src/Makefile.in: src/Makefile.am
	automake --copy --add-missing

tests/Makefile.in: tests/Makefile.am
	automake --copy --add-missing

src/config.h.in: configure
	autoheader

libtool ltmain.sh: aclocal.m4
	$(LIBTOOLIZE) --copy --force

# Need to re-run aclocal whenever m4 files are modified.
aclocal.m4: M4/*.m4
	$(ACLOCAL) $(ACLOCAL_INC)

clean:
	rm -f libtool ltmain.sh aclocal.m4 src/config.h.in config.cache config.status
	rm -rf autom4te.cache
	find . -name Makefile.in -exec rm -f {} \;
	find . -name .deps -type d -exec rm -rf {} \;

