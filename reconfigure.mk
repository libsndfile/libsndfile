#!/usr/bin/make -f

Makefile.am: configure
	automake --copy --add-missing

configure: configure.ac src/config.h.in libtool ltmain.sh
	autoconf

src/config.h.in: configure.ac libtool
	autoheader

libtool ltmain.sh: aclocal.m4
	libtoolize --copy --force
	
aclocal.m4: acinclude.m4
	aclocal

acinclude.m4:
	@echo "acinclude.m4"
	@if [ -d $(HOME)/Proj/M4 ] ; then \
		cat $(HOME)/Proj/M4/extra_largefile.m4 >acinclude.m4.new ; \
		cat $(HOME)/Proj/M4/endian.m4 >>acinclude.m4.new ; \
		cat $(HOME)/Proj/M4/lrint.m4 >>acinclude.m4.new ; \
		cat $(HOME)/Proj/M4/lrintf.m4 >>acinclude.m4.new ; \
		cat $(HOME)/Proj/M4/llrint.m4 >>acinclude.m4.new ; \
		cat $(HOME)/Proj/M4/clip_mode.m4 >>acinclude.m4.new ; \
		mv -f acinclude.m4.new acinclude.m4 ; \
	else \
		touch acinclude.m4 ; \
		fi

clean:
	rm -f libtool ltmain.sh aclocal.m4 Makefile.in src/config.h.in config.cache


# Do not edit or modify anything in this comment block.
# The arch-tag line is a file identity tag for the GNU Arch 
# revision control system.
#
# arch-tag: 2b02bfd0-d5ed-489b-a554-2bf36903cca9

