#!/usr/bin/make -f

TARGET = libsndfile-binary.tar.gz

LIBS = tmp/lib/libogg.a tmp/lib/libvorbis.a tmp/lib/libspeex.a \
		tmp/lib/liboggz.a tmp/lib/libfishsound.a

libsndfile-binary.tar.gz :

tmp/lib/libogg.a : tmp/src/lib
	



# Do not edit or modify anything in this comment block.
# The arch-tag line is a file identity tag for the GNU Arch 
# revision control system.
#
# arch-tag: 0072f040-d4a6-4f8d-b183-a0f6d3b907c7
