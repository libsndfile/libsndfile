/*
** Copyright (C) 2007 Erik de Castro Lopo <erikd@mega-nerd.com>
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
#include <unistd.h>

#include <math.h>

#include	<sndfile.h>

#include	"utils.h"
#include	"generate.h"
#include	"dft_cmp.h"

#define	SAMPLE_RATE		16000

static inline float
max_float (float a, float b)
{	return a > b ? a : b ;
} /* max_float */

static void
vorbis_test (void)
{	static float float_data [DFT_DATA_LENGTH] ;
	const char * filename = "vorbis_test.oga" ;
	SNDFILE * file ;
	SF_INFO sfinfo ;
	float max_abs = 0.0 ;
	unsigned k ;

	print_test_name ("vorbis_test", filename) ;

	/* Generate float data. */
	gen_windowed_sine_float (float_data, ARRAY_LEN (float_data), 1.0) ;

	/* Set up output file type. */
	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	sfinfo.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS ;
	sfinfo.channels = 1 ;
	sfinfo.samplerate = SAMPLE_RATE ;

	/* Write the output file. */
	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_FALSE, __LINE__) ;
	test_write_float_or_die (file, 0, float_data, ARRAY_LEN (float_data), __LINE__) ;
	sf_close (file) ;

	memset (float_data, 0, sizeof (float_data)) ;

	/* Read the file back in again. */
	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_FALSE, __LINE__) ;
	test_read_float_or_die (file, 0, float_data, ARRAY_LEN (float_data), __LINE__) ;
	sf_close (file) ;

	for (k = 0 ; k < ARRAY_LEN (float_data) ; k ++)
		max_abs = max_float (max_abs, fabs (float_data [k])) ;

	if (max_abs > 1.02)
	{	printf ("\n\n    Error : max_abs %f should be < 1.02.\n\n", max_abs) ;
		exit (1) ;
		} ;

	puts ("ok") ;
	unlink (filename) ;
} /* vorbis_test */


int
main (void)
{
	vorbis_test () ;

	return 0 ;
} /* main */
