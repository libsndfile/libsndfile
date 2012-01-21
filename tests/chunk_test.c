/*
** Copyright (C) 2003-2012 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include "sfconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include	<sndfile.h>

#include	"utils.h"

#define	BUFFER_LEN			(1 << 10)
#define LOG_BUFFER_SIZE		1024

static void	chunk_test (const char *filename, int typemajor) ;

int
main (int argc, char *argv [])
{	int		do_all = 0 ;
	int		test_count = 0 ;

	if (argc != 2)
	{	printf ("Usage : %s <test>\n", argv [0]) ;
		printf ("    Where <test> is one of the following:\n") ;
		printf ("           wav  - test adding chunks to WAV files\n") ;
		printf ("           aiff - test adding chunks to AIFF files\n") ;
		printf ("           all  - perform all tests\n") ;
		exit (1) ;
		} ;

	do_all = ! strcmp (argv [1], "all") ;

	if (do_all || ! strcmp (argv [1], "wav"))
	{	chunk_test ("chunks.wav", SF_FORMAT_WAV) ;
		chunk_test ("chunks.rifx", SF_ENDIAN_BIG | SF_FORMAT_WAV) ;
		chunk_test ("chunks.wavex", SF_FORMAT_WAVEX) ;
		test_count++ ;
		} ;

	if (do_all || ! strcmp (argv [1], "aiff"))
	{	chunk_test ("chunks.aiff", SF_FORMAT_AIFF) ;
		test_count++ ;
		} ;

	if (do_all || ! strcmp (argv [1], "caf"))
	{	chunk_test ("chunks.caf", SF_FORMAT_CAF) ;
		test_count++ ;
		} ;

	if (test_count == 0)
	{	printf ("Mono : ************************************\n") ;
		printf ("Mono : *  No '%s' test defined.\n", argv [1]) ;
		printf ("Mono : ************************************\n") ;
		return 1 ;
		} ;

	return 0 ;
} /* main */


/*============================================================================================
**	Here are the test functions.
*/

static void
chunk_test (const char *filename, int typemajor)
{	const char*		testdata = "There can be only one." ;
	SNDFILE			*file ;
	SF_INFO			sfinfo ;
	SF_CHUNK_INFO	chunk_info ;
	uint32_t		length_before ;
	int				err ;

	print_test_name ("chunk_test", filename) ;

	sfinfo.samplerate	= 44100 ;
	sfinfo.channels		= 1 ;
	sfinfo.frames		= 0 ;

	switch (typemajor)
	{	case SF_FORMAT_OGG :
			sfinfo.format = typemajor | SF_FORMAT_VORBIS ;
			break ;

		default :
			sfinfo.format = typemajor | SF_FORMAT_PCM_16 ;
			break ;
		} ;

	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_TRUE, __LINE__) ;

	/* Set up the chunk to write. */
	memset (&chunk_info, 0, sizeof (chunk_info)) ;
	snprintf (chunk_info.id, sizeof (chunk_info.id), "Test") ;
	chunk_info.id_size = 4 ;
	chunk_info.data = strdup (testdata) ;
	chunk_info.datalen = strlen (chunk_info.data) ;

	length_before = chunk_info.datalen ;

	err = sf_set_chunk (file, &chunk_info) ;
	exit_if_true (
		err != SF_ERR_NO_ERROR,
		"\n\nLine %d : sf_set_chunk returned : %s\n", __LINE__, sf_error_number (err)
		) ;

	memset (chunk_info.data, 0, chunk_info.datalen) ;
	free (chunk_info.data) ;

	sf_close (file) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_TRUE, __LINE__) ;

	memset (&chunk_info, 0, sizeof (chunk_info)) ;
	snprintf (chunk_info.id, sizeof (chunk_info.id), "Test") ;
	chunk_info.id_size = 4 ;

	err = sf_get_chunk_size (file, &chunk_info) ;
	exit_if_true (
		err != SF_ERR_NO_ERROR,
		"\n\nLine %d : sf_get_chunk_size returned : %s\n", __LINE__, sf_error_number (err)
		) ;

	exit_if_true (
		length_before > chunk_info.datalen || chunk_info.datalen - length_before > 4,
		"\n\nLine %d : Bad chunk length %u (previous length %u)\n", __LINE__, chunk_info.datalen, length_before
		) ;

	chunk_info.data = malloc (chunk_info.datalen) ;
	err = sf_get_chunk_data (file, &chunk_info) ;
	exit_if_true (
		err != SF_ERR_NO_ERROR,
		"\n\nLine %d : sf_get_chunk_data returned : %s\n", __LINE__, sf_error_number (err)
		) ;

	exit_if_true (
		memcmp (testdata, chunk_info.data, length_before),
		"\n\nLine %d : Data compare failed.\n    %s\n    %s\n\n", __LINE__, testdata, chunk_info.data
		) ;

	free (chunk_info.data) ;

	sf_close (file) ;
	unlink (filename) ;

	puts ("ok") ;
} /* chunk_test */

