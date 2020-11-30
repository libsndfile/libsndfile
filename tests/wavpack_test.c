/*
** Copyright (C) 2007-2018 Erik de Castro Lopo <erikd@mega-nerd.com>
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
#include <assert.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#else
#include "sf_unistd.h"
#endif

#include <math.h>
#include <inttypes.h>
#include <sndfile.h>

#include "utils.h"


#define	SAMPLE_RATE			48000
#define	MAX_DATA_LENGTH			262144

typedef union
{	double d [MAX_DATA_LENGTH] ;
	float f [MAX_DATA_LENGTH] ;
	int i [MAX_DATA_LENGTH] ;
	short s [MAX_DATA_LENGTH] ;
} BUFFER ;

static BUFFER data_out ;
static BUFFER data_in ;
static char str_buf [256] = { 0 } ;

static void
wavpack_short_test (int otype, int channels, int len)
{	const char * filename = "wavpack_short.wv" ;

	int len_items = len * channels ;

	SNDFILE * file ;
	SF_INFO sfinfo ;
	short seek_data [190] ;
	unsigned k ;

	sprintf (str_buf, "wavpack_short_test_%d_%d_%d", otype, channels, len) ;
	print_test_name (str_buf, filename) ;

	/* Generate float data. */
	gen_windowed_sine_float (data_out.f, len_items, 1.0 * 0x7F00) ;

	/* Convert to short. */
	for (k = 0 ; k < (unsigned) len_items ; k++)
		data_out.s [k] = lrintf (data_out.f [k]) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	/* Set up output file type. */
	sfinfo.format = SF_FORMAT_WAVPACK | otype ;
	sfinfo.channels = channels ;
	sfinfo.samplerate = SAMPLE_RATE ;

	/* Write the output file. */
	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_FALSE, __LINE__) ;
	test_write_short_or_die (file, 0, data_out.s, len_items, __LINE__) ;
	sf_close (file) ;

	/* Read the file in again. */
	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_FALSE, __LINE__) ;
	test_read_short_or_die (file, 0, data_in.s, len_items, __LINE__) ;
	sf_close (file) ;

	for (unsigned i = 0 ; i < (unsigned) len_items ; ++ i)
	{	if ((otype & SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_U8)
			assert ((data_in.s [i] / 256) == (int) (data_out.s [i] / 256)) ;
		else
			assert (data_in.s [i] == data_out.s [i]) ;
		} ;

	puts ("ok") ;

	/* Test seeking. */
	if (otype == SF_FORMAT_PCM_16)
	{	sprintf (str_buf, "wavpack_short_seek_test_%d_%d_%d", otype, channels, len) ;
		print_test_name (str_buf, filename) ;

		file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_FALSE, __LINE__) ;

		test_seek_or_die (file, 10, SEEK_SET, 10, sfinfo.channels, __LINE__) ;
		test_read_short_or_die (file, 0, seek_data, 10 * channels, __LINE__) ;
		compare_short_or_die (seek_data, data_in.s + 10 * channels, 10 * channels, __LINE__) ;

		/* Test seek to end of file. */
		test_seek_or_die (file, 0, SEEK_END, sfinfo.frames, sfinfo.channels, __LINE__) ;

		sf_close (file) ;

		puts ("ok") ;
		} ;

	unlink (filename) ;
} /* wavpack_short_test */

static void
wavpack_int_test (int otype, int channels, int len)
{	const char * filename = "wavpack_int.wv" ;

	SNDFILE * file ;
	SF_INFO sfinfo ;
	int seek_data [190] ;
	unsigned k ;

	int len_items = len * channels ;

	sprintf (str_buf, "wavpack_int_test_%d_%d_%d", otype, channels, len) ;
	print_test_name (str_buf, filename) ;

	/* Generate float data. */
	gen_windowed_sine_float (data_out.f, len_items, 1.0 * 0x7FFF0000) ;

	/* Convert to integer. */
	for (k = 0 ; k < (unsigned) len_items ; k++)
		data_out.i [k] = lrintf (data_out.f [k]) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	/* Set up output file type. */
	sfinfo.format = SF_FORMAT_WAVPACK | otype ;
	sfinfo.channels = channels ;
	sfinfo.samplerate = SAMPLE_RATE ;

	/* Write the output file. */
	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_FALSE, __LINE__) ;
	test_write_int_or_die (file, 0, data_out.i, len_items, __LINE__) ;
	sf_close (file) ;

	/* Read the file in again. */
	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_FALSE, __LINE__) ;
	test_read_int_or_die (file, 0, data_in.i, len_items, __LINE__) ;
	sf_close (file) ;

	for (unsigned i = 0 ; i < (unsigned) len_items ; ++ i)
	{	//printf ("%d | %d \n", (data_in.i [i]), (int) (data_out.i [i])) ;
		if ((otype & SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_U8)
			assert ((data_in.i [i] / 16777216) == (int) (data_out.i [i] / 16777216)) ;
		else if ((otype & SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_16)
			assert ((data_in.i [i] / 65536) == (int) (data_out.i [i] / 65536)) ;
		else if ((otype & SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_24)
			assert ((data_in.i [i] / 256) == (int) (data_out.i [i] / 256)) ;
		else
			assert (data_in.i [i] == data_out.i [i]) ;
		} ;

	puts ("ok") ;

	/* Test seeking. */
	if (otype == SF_FORMAT_PCM_32)
	{	sprintf (str_buf, "wavpack_int_seek_test_%d_%d_%d", otype, channels, len) ;
		print_test_name (str_buf, filename) ;

		file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_FALSE, __LINE__) ;

		test_seek_or_die (file, 10, SEEK_SET, 10, sfinfo.channels, __LINE__) ;
		test_read_int_or_die (file, 0, seek_data, 10 * channels, __LINE__) ;
		compare_int_or_die (seek_data, data_in.i + 10 * channels, 10 * channels, __LINE__) ;

		sf_close (file) ;

		puts ("ok") ;
		} ;

	unlink (filename) ;
} /* wavpack_int_test */

static void
wavpack_float_test (int otype, int channels, int len)
{	const char * filename = "wavpack_float.wv" ;

	SNDFILE * file ;
	SF_INFO sfinfo ;
	float seek_data [190] ;

	int len_items = len * channels ;

	sprintf (str_buf, "wavpack_float_test_%d_%d_%d", otype, channels, len) ;
	print_test_name (str_buf, filename) ;

	gen_windowed_sine_float (data_out.f, len_items, 0.95) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	/* Set up output file type. */
	sfinfo.format = SF_FORMAT_WAVPACK | otype ;
	sfinfo.channels = channels ;
	sfinfo.samplerate = SAMPLE_RATE ;

	/* Write the output file. */
	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_FALSE, __LINE__) ;
	test_write_float_or_die (file, 0, data_out.f, len_items, __LINE__) ;
	sf_close (file) ;

	/* Read the file in again. */
	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_FALSE, __LINE__) ;
	test_read_float_or_die (file, 0, data_in.f, len_items, __LINE__) ;
	sf_close (file) ;

	for (unsigned i = 0 ; i < (unsigned) len_items ; ++ i)
	{	// printf ("%f | %f \n", (data_in.f [i]), (int) (data_out.f [i])) ;
		if ((otype & SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_U8)
			assert (fabs (data_in.f [i] - data_out.f [i]) <= 1.0f / 128.0f) ;
		else if ((otype & SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_16)
			assert (fabs (data_in.f [i] - data_out.f [i]) <= 1.0f / 32768.0f) ;
		else if ((otype & SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_24)
			assert (fabs (data_in.f [i] - data_out.f [i]) <= 1.0f / 16777216.0f) ;
		else if ((otype & SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_32)
			assert (fabs (data_in.f [i] - data_out.f [i]) <= 1.0f / 2147483648.0f) ;
		else
			assert (data_in.f [i] == data_out.f [i]) ;
		} ;

	puts ("ok") ;

	/* Test seeking. */
	if (otype == SF_FORMAT_FLOAT)
	{	sprintf (str_buf, "wavpack_float_seek_test_%d_%d_%d", otype, channels, len) ;
		print_test_name (str_buf, filename) ;

		file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_FALSE, __LINE__) ;

		test_seek_or_die (file, 10, SEEK_SET, 10, sfinfo.channels, __LINE__) ;
		test_read_float_or_die (file, 0, seek_data, 10 * channels, __LINE__) ;
		compare_float_or_die (seek_data, data_in.f + 10 * channels, 10 * channels, __LINE__) ;

		sf_close (file) ;

		puts ("ok") ;
		} ;

	unlink (filename) ;
} /* wavpack_float_test */

static void
wavpack_double_test (int otype, int channels, int len)
{	const char * filename = "wavpack_double.wv" ;

	SNDFILE * file ;
	SF_INFO sfinfo ;
	double seek_data [190] ;

	int len_items = len * channels ;

	sprintf (str_buf, "wavpack_double_test_%d_%d_%d", otype, channels, len) ;
	print_test_name (str_buf, filename) ;

	gen_windowed_sine_double (data_out.d, len_items, 0.95) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	/* Set up output file type. */
	sfinfo.format = SF_FORMAT_WAVPACK | otype ;
	sfinfo.channels = channels ;
	sfinfo.samplerate = SAMPLE_RATE ;

	/* Write the output file. */
	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_FALSE, __LINE__) ;
	test_write_double_or_die (file, 0, data_out.d, len_items, __LINE__) ;
	sf_close (file) ;

	/* Read the file in again. */
	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_FALSE, __LINE__) ;
	test_read_double_or_die (file, 0, data_in.d, len_items, __LINE__) ;
	sf_close (file) ;

	for (unsigned i = 0 ; i < (unsigned) len_items ; ++ i)
	{	// printf ("%lf | %lf \n", (data_in.d [i]), (int) (data_out.d [i])) ;
		if ((otype & SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_U8)
			assert (fabs (data_in.d [i] - data_out.d [i]) <= 1.0f / 128.0f) ;
		else if ((otype & SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_16)
			assert (fabs (data_in.d [i] - data_out.d [i]) <= 1.0f / 32768.0f) ;
		else if ((otype & SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_24)
			assert (fabs (data_in.d [i] - data_out.d [i]) <= 1.0f / 16777216.0f) ;
		else if ((otype & SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_32)
			assert (fabs (data_in.d [i] - data_out.d [i]) <= 1.0f / 2147483648.0f) ;
		else
			assert (((float) data_in.d [i]) == ((float) data_out.d [i])) ;
		} ;

	puts ("ok") ;

	/* Test seeking. */
	if (otype == SF_FORMAT_FLOAT)
	{	sprintf (str_buf, "wavpack_double_test_%d_%d_%d", otype, channels, len) ;
		print_test_name (str_buf, filename) ;

		file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_FALSE, __LINE__) ;

		test_seek_or_die (file, 10, SEEK_SET, 10, sfinfo.channels, __LINE__) ;
		test_read_double_or_die (file, 0, seek_data, 10 * channels, __LINE__) ;
		compare_double_or_die (seek_data, data_in.d + 10 * channels, 10 * channels, __LINE__) ;

		sf_close (file) ;

		puts ("ok") ;
		} ;

	unlink (filename) ;
} /* wavpack_double_test */


static void
wavpack_multich_seek_test (const char * filename, int format, int channels)
{	static float data [SAMPLE_RATE] ;

	float *multichannel_out = malloc (sizeof (float) * SAMPLE_RATE * channels) ;

	SNDFILE * file ;
	SF_INFO sfinfo ;
	sf_count_t pos ;
	unsigned k ;

	sprintf (str_buf, "wavpack_multich_seek_test_%d_%d", format & SF_FORMAT_SUBMASK, channels) ;
	print_test_name (str_buf, filename) ;

	gen_windowed_sine_float (data, ARRAY_LEN (data), 0.95) ;
	for (k = 0 ; k < ARRAY_LEN (data) ; k++)
	{	for (int i = 0 ; i < channels ; ++ i)
			multichannel_out [channels * k + i] = data [i % 2 == 0 ? k : ARRAY_LEN (data) - k - 1] ;
		} ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	/* Set up output file type. */
	sfinfo.format = format ;
	sfinfo.channels = channels ;
	sfinfo.samplerate = SAMPLE_RATE ;

	/* Write the output file. */
	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_FALSE, __LINE__) ;
	test_write_float_or_die (file, 0, multichannel_out, SAMPLE_RATE * channels, __LINE__) ;
	sf_close (file) ;

	/* Open file in again for reading. */
	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_FALSE, __LINE__) ;

	/* Read in the whole file. */
	test_read_float_or_die (file, 0, multichannel_out, SAMPLE_RATE * channels, __LINE__) ;

	/* Now hammer seeking code. */
	test_seek_or_die (file, 234, SEEK_SET, 234, sfinfo.channels, __LINE__) ;
	test_readf_float_or_die (file, 0, data, 10, __LINE__) ;
	compare_float_or_die (data, multichannel_out + (234 * sfinfo.channels), 10, __LINE__) ;

	test_seek_or_die (file, 442, SEEK_SET, 442, sfinfo.channels, __LINE__) ;
	test_readf_float_or_die (file, 0, data, 10, __LINE__) ;
	compare_float_or_die (data, multichannel_out + (442 * sfinfo.channels), 10, __LINE__) ;

	test_seek_or_die (file, 12, SEEK_CUR, 442 + 10 + 12, sfinfo.channels, __LINE__) ;
	test_readf_float_or_die (file, 0, data, 10, __LINE__) ;
	compare_float_or_die (data, multichannel_out + ((442 + 10 + 12) * sfinfo.channels), 10, __LINE__) ;

	test_seek_or_die (file, 12, SEEK_CUR, 442 + 20 + 24, sfinfo.channels, __LINE__) ;
	test_readf_float_or_die (file, 0, data, 10, __LINE__) ;
	compare_float_or_die (data, multichannel_out + ((442 + 20 + 24) * sfinfo.channels), 10, __LINE__) ;

	pos = 500 - sfinfo.frames ;
	test_seek_or_die (file, pos, SEEK_END, 500, sfinfo.channels, __LINE__) ;
	test_readf_float_or_die (file, 0, data, 10, __LINE__) ;
	compare_float_or_die (data, multichannel_out + (500 * sfinfo.channels), 10, __LINE__) ;

	pos = 10 - sfinfo.frames ;
	test_seek_or_die (file, pos, SEEK_END, 10, sfinfo.channels, __LINE__) ;
	test_readf_float_or_die (file, 0, data, 10, __LINE__) ;
	compare_float_or_die (data, multichannel_out + (10 * sfinfo.channels), 10, __LINE__) ;

	sf_close (file) ;

	puts ("ok") ;
	unlink (filename) ;

	free (multichannel_out) ;
} /* wavpack_stereo_seek_test */

int
main (void)
{
	static int otype_list [] = { SF_FORMAT_PCM_U8, SF_FORMAT_PCM_16, SF_FORMAT_PCM_24, SF_FORMAT_PCM_32, SF_FORMAT_FLOAT } ;
	static int len_list [] = { 1233, 2048, 6000, 8192, 9001 } ;
	if (HAVE_WAVPACK)
	{	for (int channels = 1 ; channels <= 19 ; ++ channels)
		{	for (unsigned i_otype = 0 ; i_otype < ARRAY_LEN (otype_list) ; ++ i_otype)
			{	int otype = otype_list [i_otype] ;
				for (unsigned i_len = 0 ; i_len < ARRAY_LEN (len_list) ; ++ i_len)
				{	int len = len_list [i_len] ;
					wavpack_short_test (otype, channels, len) ;
					wavpack_int_test (otype, channels, len) ;
					wavpack_float_test (otype, channels, len) ;
					wavpack_double_test (otype, channels, len) ;
					} ;
				wavpack_multich_seek_test ("wavpack_seek.wv", SF_FORMAT_WAVPACK | otype, channels) ;
				} ;
			} ;
		}
	else
		puts ("    No Wavpack tests because Wavpack support was not compiled in.") ;

	return 0 ;
} /* main */
