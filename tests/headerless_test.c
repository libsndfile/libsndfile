/*
** Copyright (C) 1999-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sndfile.h>

#include "utils.h"

#define	BUFFER_SIZE		(100)

int
main (void)
{	static	short	buffer [BUFFER_SIZE] ;
	SNDFILE		*file ;
	SF_INFO		sfinfo ;
	int			k, filetype ;
	const char	*filename = "headerless.wav" ;

	printf ("    header-less test : ") ;
	fflush (stdout) ;

	for (k = 0 ; k < BUFFER_SIZE ; k++)
		buffer [k] = k ;

	filetype = SF_FORMAT_WAV | SF_FORMAT_PCM_16 ;

	sfinfo.samplerate	= 32000 ;
	sfinfo.frames		= 123456789 ; /* Wrong length. Library should correct this on sf_close. */
	sfinfo.channels		= 1 ;
	sfinfo.format		= filetype ;

	if (! (file = sf_open (filename, SFM_WRITE, &sfinfo)))
	{	printf ("Line %d: sf_open_write failed with error : ", __LINE__) ;
		fflush (stdout) ;
		puts (sf_strerror (NULL)) ;
		exit (1) ;
		} ;

	if ((k = sf_write_short (file, buffer, BUFFER_SIZE)) != BUFFER_SIZE)
	{	printf ("Line %d: sf_write_short failed with short write (%d => %d).\n", __LINE__, BUFFER_SIZE, k) ;
		fflush (stdout) ;
		puts (sf_strerror (file)) ;
		exit (1) ;
		} ;

	sf_close (file) ;

	memset (buffer, 0, sizeof (buffer)) ;

	/* Read as RAW but get the bit width and endian-ness correct. */
	sfinfo.format = filetype = SF_ENDIAN_LITTLE | SF_FORMAT_RAW | SF_FORMAT_PCM_16 ;

	if (! (file = sf_open (filename, SFM_READ, &sfinfo)))
	{	printf ("Line %d: sf_open_read failed with error : ", __LINE__) ;
		fflush (stdout) ;
		puts (sf_strerror (NULL)) ;
		exit (1) ;
		} ;

	if (sfinfo.format != filetype)
	{	printf ("Line %d: Returned format incorrect (0x%08X => 0x%08X).\n", __LINE__, filetype, sfinfo.format) ;
		exit (1) ;
		} ;

	if (sfinfo.frames < BUFFER_SIZE)
	{	printf ("Line %d: Incorrect number of.frames in file. (%d => %ld)\n", __LINE__, BUFFER_SIZE, SF_COUNT_TO_LONG (sfinfo.frames)) ;
		exit (1) ;
		} ;

	if (sfinfo.channels != 1)
	{	printf ("Line %d: Incorrect number of channels in file.\n", __LINE__) ;
		exit (1) ;
		} ;

	check_log_buffer_or_die (file, __LINE__) ;

	if ((k = sf_read_short (file, buffer, BUFFER_SIZE)) != BUFFER_SIZE)
	{	printf ("Line %d: short read (%d).\n", __LINE__, k) ;
		exit (1) ;
		} ;

	for (k = 0 ; k < BUFFER_SIZE - 22 ; k++)
		if (buffer [k + 22] != k)
		{	printf ("Line %d: Incorrect sample (#%d : 0x%x => 0x%x).\n", __LINE__, k, k, buffer [k]) ;
			exit (1) ;
			} ;

	printf ("ok\n") ;
	unlink (filename) ;

	return 0 ;
} /* main */

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 0820bc1a-c396-4e66-9997-096999b0bc40
*/
