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

#include <math.h>

#include	<sndfile.h>

#include	"utils.h"
#include	"generate.h"

#define	SAMPLE_RATE			44100
#define	DATA_LENGTH			(SAMPLE_RATE / 8)


static void
ogg_float_test (void)
{	const char * filename = "vorbis.oga" ;
	static float data_out [DATA_LENGTH] ;
	static float data_in [DATA_LENGTH] ;

	SNDFILE * file ;
	SF_INFO sfinfo ;

	print_test_name ("ogg_float_test", filename) ;

	gen_windowed_sine_float (data_out, ARRAY_LEN (data_out), 0.95) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	/* Set up output file type. */
	sfinfo.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS ;
	sfinfo.channels = 1 ;
	sfinfo.samplerate = SAMPLE_RATE ;

	/* Write the output file. */
	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_FALSE, __LINE__) ;
	test_write_float_or_die (file, 0, data_out, ARRAY_LEN (data_out), __LINE__) ;
	sf_close (file) ;

	/* Read the file in again. */
	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_FALSE, __LINE__) ;
	test_read_float_or_die (file, 0, data_in, ARRAY_LEN (data_in), __LINE__) ;
	sf_close (file) ;

	puts ("ok") ;

	/* Test seeking. */
	print_test_name ("ogg_seek_test", filename) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_FALSE, __LINE__) ;
	test_seek_or_die (file, 10, SEEK_SET, 10, sfinfo.channels, __LINE__) ;
	sf_close (file) ;

	puts ("ok") ;

	/*-unlink (filename) ;-*/
} /* ogg_float_test */

static void
ogg_double_test (void)
{	const char * filename = "vorbis.oga" ;
	static double data_out [DATA_LENGTH] ;
	static double data_in [DATA_LENGTH] ;

	SNDFILE * file ;
	SF_INFO sfinfo ;

	print_test_name ("ogg_double_test", filename) ;

	gen_windowed_sine_double (data_out, ARRAY_LEN (data_out), 0.95) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	/* Set up output file type. */
	sfinfo.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS ;
	sfinfo.channels = 1 ;
	sfinfo.samplerate = SAMPLE_RATE ;

	/* Write the output file. */
	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_FALSE, __LINE__) ;
	test_write_double_or_die (file, 0, data_out, ARRAY_LEN (data_out), __LINE__) ;
	sf_close (file) ;

	/* Read the file in again. */
	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_FALSE, __LINE__) ;
	test_read_double_or_die (file, 0, data_in, ARRAY_LEN (data_in), __LINE__) ;
	sf_close (file) ;

	puts ("ok") ;

	/* Test seeking. */
	print_test_name ("ogg_seek_test", filename) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_FALSE, __LINE__) ;
	test_seek_or_die (file, 10, SEEK_SET, 10, sfinfo.channels, __LINE__) ;
	sf_close (file) ;

	puts ("ok") ;

	/*-unlink (filename) ;-*/
} /* ogg_double_test */

int
main (void)
{
	ogg_float_test () ;
	ogg_double_test () ;

	return 0 ;
} /* main */
