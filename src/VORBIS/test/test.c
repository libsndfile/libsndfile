/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2007             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: vorbis coded test suite using vorbisfile
 last mod: $Id: test.c 13293 2007-07-24 00:09:47Z erikd $

 ********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "util.h"
#include "write_read.h"

#define ARRAY_LEN(x)   (sizeof(x)/sizeof(x[0]))

#define MAX(a,b)	((a) > (b) ? (a) : (b))


static int mono_short_data_test (const char * filename, int srate) ;
static int mono_dual_short_test (const char * filename, int srate) ;

int
main (void)
{	int total_errors = 0 ;

	/* Do safest and most used sample rates first. */
	int sample_rates [] = { 44100, 48000, 32000, 22050, 16000, 96000, 88200, 8000 } ;
	unsigned k ;

	for (k = 0 ; k < ARRAY_LEN (sample_rates) ; k ++)
	{	char filename [64] ;
		int errors = 0 ;

		snprintf (filename, sizeof (filename), "vorbis_%u.oga", sample_rates [k]) ;

		printf ("    %-20s : ", filename) ;
		fflush (stdout) ;

		if (mono_short_data_test (filename, sample_rates [k]))
			errors ++ ;
		else if (mono_dual_short_test (filename, sample_rates [k]))
			errors ++ ;

		if (errors == 0)
		{	puts ("ok") ;
			remove (filename) ;
			} ;

		total_errors += errors ;
  		} ;

	return 0 ;
} /* main */

static int
mono_short_data_test (const char * filename, int srate)
{	static float data_out [2048] ;
	static float data_in [2048] ;

	float max_abs = 0.0 ;
	unsigned k, len ;

	memset (data_out, 0, sizeof (data_out)) ;
	len = ARRAY_LEN (data_out) / 2 ;
	for (k = 0 ; k < len ; k++)
    {    data_out [k] = sin (2.0 * k * M_PI * 1.0 / 32.0 + 0.4) ;

        /* Apply Hanning Window. */
        data_out [k] *= 0.95 * (0.5 - 0.5 * cos (2.0 * M_PI * k / (len - 1))) ;
        } ;

	/* Set to known value. */
	set_data_in (data_in, ARRAY_LEN (data_in), 3.141) ;

	write_vorbis_data_or_die (filename, srate, data_out, ARRAY_LEN (data_out)) ;
	read_vorbis_data_or_die (filename, srate, data_in, ARRAY_LEN (data_in)) ;

	for (k = 0 ; k < ARRAY_LEN (data_in) ; k++)
	{	float temp = fabs (data_in [k]) ;
    	max_abs = MAX (max_abs, temp) ;
  		} ;

	if (max_abs < 0.9)
	{	printf ("%s : max_abs (%f) too small.\n", __func__, max_abs) ;
    	return 1 ;
  		}
	else if (max_abs > 1.0)
	{	printf ("%s : max_abs (%f) too big.\n", __func__, max_abs) ;
    	return 1 ;
  		} ;

	return 0 ;
} /* mono_short_data_test */

static int
mono_dual_short_test (const char * filename, int srate)
{	static float data_out [4096] ;
	static float data_in [4096] ;

	float max_abs = 0.0 ;
	unsigned k, len ;

	memset (data_out, 0, sizeof (data_out)) ;
	len = ARRAY_LEN (data_out) ;
	for (k = 0 ; k < len ; k++)
    {    data_out [k] = 0.5 * (sin (2.0 * k * M_PI * 1.0 / 32.0) + sin (2.0 * k * M_PI * 1.0 / 64.0)) ;

        /* Apply Hanning Window. */
        data_out [k] *= 0.5 - 0.5 * cos (2.0 * M_PI * k / (len - 1)) ;
        } ;

	/* Set to known value. */
	set_data_in (data_in, ARRAY_LEN (data_in), 3.141) ;

	write_vorbis_data_or_die (filename, srate, data_out, ARRAY_LEN (data_out)) ;
	read_vorbis_data_or_die (filename, srate, data_in, ARRAY_LEN (data_in)) ;

	for (k = 0 ; k < ARRAY_LEN (data_in) ; k++)
	{	float temp = fabs (data_in [k]) ;
    	max_abs = MAX (max_abs, temp) ;
  		} ;

	if (max_abs < 0.85)
	{	printf ("%s : max_abs (%f) too small.\n", __func__, max_abs) ;
    	return 1 ;
  		}
	else if (max_abs > 0.9)
	{	printf ("%s : max_abs (%f) too big.\n", __func__, max_abs) ;
    	return 1 ;
  		} ;

	return 0 ;
} /* mono_dual_short_test */

