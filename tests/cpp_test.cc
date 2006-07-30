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
#include <cstring>

#include <sndfile.hh>

#include "utils.h"

static short	sbuffer [100] ;
static int 		ibuffer [100] ;
static float	fbuffer [100] ;
static double	dbuffer [100] ;

static void
create_file (const char * filename, int format)
{	Sndfile file ;

	file.open (filename, SFM_WRITE, format, 2, 48000) ;

	file.setString (SF_STR_TITLE, filename) ;

	/* Item write. */
	file.write (sbuffer, ARRAY_LEN (sbuffer)) ;
	file.write (ibuffer, ARRAY_LEN (ibuffer)) ;
	file.write (fbuffer, ARRAY_LEN (fbuffer)) ;
	file.write (dbuffer, ARRAY_LEN (dbuffer)) ;

	/* Frame write. */
	file.writef (sbuffer, ARRAY_LEN (sbuffer) / file.channels ()) ;
	file.writef (ibuffer, ARRAY_LEN (ibuffer) / file.channels ()) ;
	file.writef (fbuffer, ARRAY_LEN (fbuffer) / file.channels ()) ;
	file.writef (dbuffer, ARRAY_LEN (dbuffer) / file.channels ()) ;

	/*
	**	An explict close() call is not really necessary as the Sndfile 
	**	destructor closes the file anyway.
	*/
	file.close () ;
} /* create_file */

static void
read_file (const char * filename, int format)
{	Sndfile file ;
	const char *title ;
	sf_count_t count ;

	if (file)
	{	printf ("\n\n%s %d : Error : should not be here.\n\n", __func__, __LINE__) ;
		exit (1) ;
		} ;

	file.open (filename) ;

	if (! file)
	{	printf ("\n\n%s %d : Error : should not be here.\n\n", __func__, __LINE__) ;
		exit (1) ;
		} ;

	if (file.format () != format)
	{	printf ("\n\n%s %d : Error : format 0x%08x should be 0x%08x.\n\n", __func__, __LINE__, file.format (), format) ;
		exit (1) ;
		} ;

	if (file.channels () != 2)
	{	printf ("\n\n%s %d : Error : channels %d should be 2.\n\n", __func__, __LINE__, file.channels ()) ;
		exit (1) ;
		} ;

	if (file.frames () != ARRAY_LEN (sbuffer) * 4)
	{	printf ("\n\n%s %d : Error : frames %ld should be %d.\n\n", __func__, __LINE__,
				SF_COUNT_TO_LONG (file.frames ()), ARRAY_LEN (sbuffer) * 4 / 2) ;
		exit (1) ;
		} ;

	title = file.getString (SF_STR_TITLE) ;

	if (title == NULL)
	{	printf ("\n\n%s %d : Error : No title.\n\n", __func__, __LINE__) ;
		exit (1) ;
		} ;

	if (strcmp (filename, title) != 0)
	{	printf ("\n\n%s %d : Error : title '%s' should be '%s'\n\n", __func__, __LINE__, title, filename) ;
		exit (1) ;
		} ;

	/* Item read. */
	file.read (sbuffer, ARRAY_LEN (sbuffer)) ;
	file.read (ibuffer, ARRAY_LEN (ibuffer)) ;
	file.read (fbuffer, ARRAY_LEN (fbuffer)) ;
	file.read (dbuffer, ARRAY_LEN (dbuffer)) ;

	/* Frame read. */
	file.readf (sbuffer, ARRAY_LEN (sbuffer) / file.channels ()) ;
	file.readf (ibuffer, ARRAY_LEN (ibuffer) / file.channels ()) ;
	file.readf (fbuffer, ARRAY_LEN (fbuffer) / file.channels ()) ;
	file.readf (dbuffer, ARRAY_LEN (dbuffer) / file.channels ()) ;

	count = file.seek (file.frames () - 10, SEEK_SET) ;
	if (count != file.frames () - 10)
	{	printf ("\n\n%s %d : Error : offset (%ld) should be %ld\n\n", __func__, __LINE__,
				SF_COUNT_TO_LONG (count), SF_COUNT_TO_LONG (file.frames () - 10)) ;
		exit (1) ;
		} ;

	count = file.read (sbuffer, ARRAY_LEN (sbuffer)) ;
	if (count != 10 * file.channels ())
	{	printf ("\n\n%s %d : Error : count (%ld) should be %ld\n\n", __func__, __LINE__,
				SF_COUNT_TO_LONG (count), SF_COUNT_TO_LONG (10 * file.channels ())) ;
		exit (1) ;
		} ;

	/*
	**	An explict close() call is not really necessary as the Sndfile 
	**	destructor closes the file anyway.
	*/
	file.close () ;
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

/*
** Do not edit or modify anything in this comment block.
** The following line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: 06e48ee6-b19d-4453-9999-a5cf2d7bf0b6
*/
