#!/usr/bin/make -f

Makefile.am: configure
	automake --copy --add-missing

configure: configure.ac src/config.h.in libtool ltmain.sh
	autoconf

src/config.h.in: configure.ac libtool
	autoheader

libtool ltmain.sh: aclocal.m4
	libtoolize --copy --force
	
aclocal.m4:
	aclocal

clean:
	rm -f libtool ltmain.sh aclocal.m4 Makefile.in src/config.h.in config.cache


# Do not edit or modify anything in this comment block.
# The arch-tag line is a file identity tag for the GNU Arch 
# revision control system.
#
# arch-tag: 2b02bfd0-d5ed-489b-a554-2bf36903cca9

