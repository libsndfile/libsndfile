#!/usr/bin/make -f

config.status: configure
	./configure

configure: configure.ac Makefile.am src/config.h.in libtool ltmain.sh
	automake --copy --add-missing
	autoconf

src/config.h.in: configure.ac libtool
	autoheader

libtool ltmain.sh: aclocal.m4
	libtoolize --copy --force
	
# Need to re-run aclocal whenever acinclude.m4 is modified.
aclocal.m4: acinclude.m4
	aclocal

clean:
	rm -f libtool ltmain.sh aclocal.m4 Makefile.in src/config.h.in config.cache


# Do not edit or modify anything in this comment block.
# The arch-tag line is a file identity tag for the GNU Arch 
# revision control system.
#
# arch-tag: 2b02bfd0-d5ed-489b-a554-2bf36903cca9

