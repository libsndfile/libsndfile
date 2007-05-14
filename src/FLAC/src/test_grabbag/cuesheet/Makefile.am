#  test_cuesheet - Simple tester for cuesheet routines in grabbag
#  Copyright (C) 2002,2003,2004,2005,2006,2007  Josh Coalson
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

INCLUDES = -I$(top_srcdir)/src/FLAC/include

noinst_PROGRAMS = test_cuesheet
test_cuesheet_SOURCES = \
	main.c
test_cuesheet_LDADD = \
	$(top_builddir)/src/FLAC/src/share/grabbag/libgrabbag.la \
	$(top_builddir)/src/FLAC/src/share/replaygain_anal/libreplaygain_analysis.la \
	$(top_builddir)/src/FLAC/src/libFLAC/libFLAC.la \
	$(top_builddir)/src/OGG/libogg.la \
	-lm

CLEANFILES = $(wildcard *.raw)
