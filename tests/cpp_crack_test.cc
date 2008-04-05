/*
** Copyright (C) 2006 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <cstdio>
#include <cstdlib>
#include <string>

#include <sndfile.hh>

#include "utils.h"

static short	sbuffer [100] ;
static int 		ibuffer [100] ;
static float	fbuffer [100] ;
static double	dbuffer [100] ;

static void
create_file (const std::string& filename, int format)
{	Sndfile::Handle file (filename, SFM_WRITE, format, 2, 48000) ;

	setString (file, SF_STR_TITLE, filename) ;

	/* Item write. */
	write  (file, sbuffer, ARRAY_LEN (sbuffer)) ;
	write  (file, ibuffer, ARRAY_LEN (ibuffer)) ;
	write  (file, fbuffer, ARRAY_LEN (fbuffer)) ;
	write  (file, dbuffer, ARRAY_LEN (dbuffer)) ;

	/* Frame write. */
	writef (file, sbuffer, ARRAY_LEN (sbuffer) / channels (file)) ;
	writef (file, ibuffer, ARRAY_LEN (ibuffer) / channels (file)) ;
	writef (file, fbuffer, ARRAY_LEN (fbuffer) / channels (file)) ;
	writef (file, dbuffer, ARRAY_LEN (dbuffer) / channels (file)) ;

	/*
	**	An explicit close() call is not necessary as the
	**      Sndfile::Handle destructor closes the file anyway.
	*/
} /* create_file */

static void
read_file (const std::string& filename, int frmt)
{	Sndfile::Handle file ;
	std::string title ;
	sf_count_t count ;

	file = Sndfile::open (filename) ;
	
	if (format (file) != frmt)
	{	printf ("\n\n%s %d : Error : format 0x%08x should be 0x%08x.\n\n", __func__, __LINE__, format (file), frmt) ;
		exit (1) ;
		} ;

	if (channels (file) != 2)
	{	printf ("\n\n%s %d : Error : channels %d should be 2.\n\n", __func__, __LINE__, channels (file)) ;
		exit (1) ;
		} ;

	if (frames (file) != ARRAY_LEN (sbuffer) * 4)
	{	printf ("\n\n%s %d : Error : frames %ld should be %d.\n\n", __func__, __LINE__,
				SF_COUNT_TO_LONG (frames (file)), ARRAY_LEN (sbuffer) * 4 / 2) ;
		exit (1) ;
		} ;

	title = getString (file, SF_STR_TITLE) ;

	if (title == "")
	{	printf ("\n\n%s %d : Error : No title.\n\n", __func__, __LINE__) ;
		exit (1) ;
		} ;

	if (filename != title)
	{	printf ("\n\n%s %d : Error : title '%s' should be '%s'\n\n", __func__, __LINE__, title.c_str(), filename.c_str()) ;
		exit (1) ;
		} ;

	/* Item read. */
	read  (file, sbuffer, ARRAY_LEN (sbuffer)) ;
	read  (file, ibuffer, ARRAY_LEN (ibuffer)) ;
	read  (file, fbuffer, ARRAY_LEN (fbuffer)) ;
	read  (file, dbuffer, ARRAY_LEN (dbuffer)) ;
	
	/* Frame read. */
	readf (file, sbuffer, ARRAY_LEN (sbuffer) / channels (file)) ;
	readf (file, ibuffer, ARRAY_LEN (ibuffer) / channels (file)) ;
	readf (file, fbuffer, ARRAY_LEN (fbuffer) / channels (file)) ;
	readf (file, dbuffer, ARRAY_LEN (dbuffer) / channels (file)) ;

	count = seek (file, frames (file) - 10, SEEK_SET) ;
	if (count != frames (file) - 10)
	{	printf ("\n\n%s %d : Error : offset (%ld) should be %ld\n\n", __func__, __LINE__,
				SF_COUNT_TO_LONG (count), SF_COUNT_TO_LONG (frames (file) - 10)) ;
		exit (1) ;
		} ;

	count = read (file, sbuffer, ARRAY_LEN (sbuffer)) ;
	if (count != 10 * channels (file))
	{	printf ("\n\n%s %d : Error : count (%ld) should be %ld\n\n", __func__, __LINE__,
				SF_COUNT_TO_LONG (count), SF_COUNT_TO_LONG (10 * channels (file))) ;
		exit (1) ;
		} ;

	/*
	**	An explicit close() call is not necessary as the
	**	Sndfile::Handle destructor closes the file anyway.
	*/
} /* create_file */

int
main (void)
{	const char * filename = "cpp_test.wav" ;

	print_test_name ("CeePlusPlus test", filename) ;

	create_file (filename, SF_FORMAT_WAV | SF_FORMAT_PCM_16) ;
	read_file (filename, SF_FORMAT_WAV | SF_FORMAT_PCM_16) ;

	remove (filename) ;
	puts ("ok") ;
	return 0 ;
} /* main */

