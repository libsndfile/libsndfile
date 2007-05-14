#  test_seeking - Seeking tester for libFLAC
#  Copyright (C) 2004,2005,2006,2007  Josh Coalson
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

AM_CFLAGS = -I$(top_srcdir)/src/FLAC/include -I$(top_srcdir)/src/OGG/include

INCLUDES = 

noinst_PROGRAMS = test_seeking
test_seeking_LDADD = \
	$(top_builddir)/src/FLAC/src/libFLAC/libFLAC.la \
	$(top_builddir)/src/OGG/libogg.la \
	-lm
test_seeking_SOURCES = \
	main.c
