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
/* The checks below use assert (), keep them in release builds. */
#undef NDEBUG
#include <assert.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#else
#include "sf_unistd.h"
#endif

#include <math.h>
#include <inttypes.h>
#include <sndfile.h>

#if HAVE_WAVPACK
#include <wavpack/wavpack.h>
#endif

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
	unsigned k, i ;

	snprintf (str_buf, sizeof (str_buf), "wavpack_short_test_%d_%d_%d", otype, channels, len) ;
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
	assert (sfinfo.format == (SF_FORMAT_WAVPACK | otype)) ;

	test_read_short_or_die (file, 0, data_in.s, len_items, __LINE__) ;
	sf_close (file) ;

	for (i = 0 ; i < (unsigned) len_items ; ++ i)
	{	int subformat = otype & SF_FORMAT_SUBMASK ;
		if (subformat == SF_FORMAT_PCM_U8 || subformat == SF_FORMAT_PCM_S8)
			assert ((data_in.s [i] >> 8) == (data_out.s [i] >> 8)) ;
		else if (subformat == SF_FORMAT_FLOAT)
			assert (abs (data_in.s [i] - data_out.s [i]) <= 1) ;
		else
			assert (data_in.s [i] == data_out.s [i]) ;
		} ;

	puts ("ok") ;

	/* Test seeking. */
	if (otype == SF_FORMAT_PCM_16)
	{	snprintf (str_buf, sizeof (str_buf), "wavpack_short_seek_test_%d_%d_%d", otype, channels, len) ;
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
	unsigned k, i ;

	int len_items = len * channels ;

	snprintf (str_buf, sizeof (str_buf), "wavpack_int_test_%d_%d_%d", otype, channels, len) ;
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
	assert (sfinfo.format == (SF_FORMAT_WAVPACK | otype)) ;

	test_read_int_or_die (file, 0, data_in.i, len_items, __LINE__) ;
	sf_close (file) ;

	for (i = 0 ; i < (unsigned) len_items ; ++ i)
	{	int subformat = otype & SF_FORMAT_SUBMASK ;
		if (subformat == SF_FORMAT_PCM_U8 || subformat == SF_FORMAT_PCM_S8)
			assert ((data_in.i [i] >> 24) == (data_out.i [i] >> 24)) ;
		else if (subformat == SF_FORMAT_PCM_16)
			assert ((data_in.i [i] >> 16) == (data_out.i [i] >> 16)) ;
		else if (subformat == SF_FORMAT_PCM_24)
			assert ((data_in.i [i] >> 8) == (data_out.i [i] >> 8)) ;
		else if (subformat == SF_FORMAT_FLOAT)
			assert (abs (data_in.i [i] - data_out.i [i]) <= 1) ;
		else
			assert (data_in.i [i] == data_out.i [i]) ;
		} ;

	puts ("ok") ;

	/* Test seeking. */
	if (otype == SF_FORMAT_PCM_32)
	{	snprintf (str_buf, sizeof (str_buf), "wavpack_int_seek_test_%d_%d_%d", otype, channels, len) ;
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
	unsigned i ;

	int len_items = len * channels ;

	snprintf (str_buf, sizeof (str_buf), "wavpack_float_test_%d_%d_%d", otype, channels, len) ;
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
	assert (sfinfo.format == (SF_FORMAT_WAVPACK | otype)) ;

	test_read_float_or_die (file, 0, data_in.f, len_items, __LINE__) ;
	sf_close (file) ;

	for (i = 0 ; i < (unsigned) len_items ; ++ i)
	{	int subformat = otype & SF_FORMAT_SUBMASK ;
		if (subformat == SF_FORMAT_PCM_U8 || subformat == SF_FORMAT_PCM_S8)
			assert (fabs (data_in.f [i] - data_out.f [i]) <= 1.5f / 128.0f) ;
		else if (subformat == SF_FORMAT_PCM_16)
			assert (fabs (data_in.f [i] - data_out.f [i]) <= 1.5f / 32768.0f) ;
		else if (subformat == SF_FORMAT_PCM_24)
			assert (fabs (data_in.f [i] - data_out.f [i]) <= 1.5f / 8388608.0f) ;
		else if (subformat == SF_FORMAT_PCM_32)
			assert (fabs (data_in.f [i] - data_out.f [i]) <= 1.5f / 2147483648.0f) ;
		else
			assert (data_in.f [i] == data_out.f [i]) ;
		} ;

	puts ("ok") ;

	/* Test seeking. */
	if (otype == SF_FORMAT_FLOAT)
	{	snprintf (str_buf, sizeof (str_buf), "wavpack_float_seek_test_%d_%d_%d", otype, channels, len) ;
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
	unsigned i ;

	int len_items = len * channels ;

	snprintf (str_buf, sizeof (str_buf), "wavpack_double_test_%d_%d_%d", otype, channels, len) ;
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
	assert (sfinfo.format == (SF_FORMAT_WAVPACK | otype)) ;

	test_read_double_or_die (file, 0, data_in.d, len_items, __LINE__) ;
	sf_close (file) ;

	for (i = 0 ; i < (unsigned) len_items ; ++ i)
	{	int subformat = otype & SF_FORMAT_SUBMASK ;
		if (subformat == SF_FORMAT_PCM_U8 || subformat == SF_FORMAT_PCM_S8)
			assert (fabs (data_in.d [i] - data_out.d [i]) <= 1.5f / 128.0f) ;
		else if (subformat == SF_FORMAT_PCM_16)
			assert (fabs (data_in.d [i] - data_out.d [i]) <= 1.5f / 32768.0f) ;
		else if (subformat == SF_FORMAT_PCM_24)
			assert (fabs (data_in.d [i] - data_out.d [i]) <= 1.5f / 8388608.0f) ;
		else if (subformat == SF_FORMAT_PCM_32)
			assert (fabs (data_in.d [i] - data_out.d [i]) <= 1.5f / 2147483648.0f) ;
		else
			assert (((float) data_in.d [i]) == ((float) data_out.d [i])) ;
		} ;

	puts ("ok") ;

	/* Test seeking. */
	if (otype == SF_FORMAT_FLOAT)
	{	snprintf (str_buf, sizeof (str_buf), "wavpack_double_seek_test_%d_%d_%d", otype, channels, len) ;
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

	snprintf (str_buf, sizeof (str_buf), "wavpack_multich_seek_test_%d_%d", format & SF_FORMAT_SUBMASK, channels) ;
	print_test_name (str_buf, filename) ;

	gen_windowed_sine_float (data, ARRAY_LEN (data), 0.95) ;
	for (k = 0 ; k < ARRAY_LEN (data) ; k++)
	{	int i ;
		for (i = 0 ; i < channels ; ++ i)
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
} /* wavpack_multich_seek_test */

#if HAVE_WAVPACK

/* One bounded memory file exercises both normal and failing virtual I/O. */
typedef struct
{	unsigned char data [128 * 1024] ;
	sf_count_t offset, length, reported_length ;
	int sequential, fail_seek, short_write ;
} WAVPACK_TEST_IO ;

static sf_count_t
wavpack_test_length (void *user_data)
{	WAVPACK_TEST_IO *io = user_data ;
	return io->reported_length ? io->reported_length : io->length ;
} /* wavpack_test_length */

static sf_count_t
wavpack_test_seek (sf_count_t offset, int whence, void *user_data)
{	WAVPACK_TEST_IO *io = user_data ;
	sf_count_t target = whence == SEEK_SET ? offset : offset + (whence == SEEK_CUR ? io->offset : io->length) ;
	if (target < 0 || io->fail_seek || (io->sequential && target != io->offset))
		return -1 ;
	return io->offset = target ;
} /* wavpack_test_seek */

static sf_count_t
wavpack_test_read (void *ptr, sf_count_t count, void *user_data)
{	WAVPACK_TEST_IO *io = user_data ;
	if (io->offset >= io->length)
		return 0 ;
	if (count > io->length - io->offset)
		count = io->length - io->offset ;
	memcpy (ptr, io->data + io->offset, (size_t) count) ;
	io->offset += count ;
	return count ;
} /* wavpack_test_read */

static sf_count_t
wavpack_test_write (const void *ptr, sf_count_t count, void *user_data)
{	WAVPACK_TEST_IO *io = user_data ;
	if (io->offset >= SIGNED_SIZEOF (io->data))
		return 0 ;
	if (count > SIGNED_SIZEOF (io->data) - io->offset)
		count = SIGNED_SIZEOF (io->data) - io->offset ;
	if (io->short_write && count > 0)
		count-- ;
	memcpy (io->data + io->offset, ptr, (size_t) count) ;
	io->offset += count ;
	if (io->offset > io->length)
		io->length = io->offset ;
	return count ;
} /* wavpack_test_write */

static sf_count_t
wavpack_test_tell (void *user_data)
{	return ((WAVPACK_TEST_IO *) user_data)->offset ;
} /* wavpack_test_tell */

static SF_VIRTUAL_IO wavpack_test_vio =
{	wavpack_test_length, wavpack_test_seek, wavpack_test_read, wavpack_test_write, wavpack_test_tell } ;

static int
wavpack_test_block_output (void *user_data, void *data, int32_t bytes)
{	return wavpack_test_write (data, bytes, user_data) == bytes ;
} /* wavpack_test_block_output */

/* Generate formats that libsndfile's encoder cannot produce itself. */
static void
wavpack_test_fixture (WAVPACK_TEST_IO *io, WavpackConfig *config, int32_t *samples,
	uint32_t frames, int64_t total_frames, const char *const *tags)
{	WavpackContext *context ;
	memset (io, 0, sizeof (*io)) ;
	context = WavpackOpenFileOutput (wavpack_test_block_output, io, NULL) ;
	assert (context != NULL) ;
	assert (WavpackSetConfiguration64 (context, config, total_frames, NULL)) ;
	assert (WavpackPackInit (context)) ;
	assert (WavpackPackSamples (context, samples, frames)) ;
	assert (WavpackFlushSamples (context)) ;
	if (tags)
	{	for (int k = 0 ; tags [k] ; k += 2)
			assert (WavpackAppendTagItem (context, tags [k], tags [k + 1], (int) strlen (tags [k + 1]))) ;
		assert (WavpackWriteTag (context)) ;
		} ;
	WavpackCloseFile (context) ;
	io->offset = 0 ;
} /* wavpack_test_fixture */

static void
wavpack_stream_test (void)
{	static WAVPACK_TEST_IO io ;
	WavpackConfig config = { 0 } ;
	int32_t samples [48] ;
	SNDFILE *file ;
	SF_INFO info ;
	SF_VIRTUAL_IO vio ;
	sf_count_t count, first_block, second_block ;
	unsigned k, item_size ;

	print_test_name ("wavpack_stream_test", "memory") ;
	config.num_channels = 1 ;
	config.sample_rate = SAMPLE_RATE ;
	config.block_samples = 16 ;
	/* Exercise ordinary conversions and the PCM32 / float fast paths. */
	for (int format = 0 ; format < 3 ; format++)
	{	config.bits_per_sample = format == 0 ? 16 : 32 ;
		config.bytes_per_sample = config.bits_per_sample / 8 ;
		config.float_norm_exp = format == 2 ? 127 : 0 ;
		for (k = 0 ; k < ARRAY_LEN (samples) ; k++)
			samples [k] = format == 2 ? 0x3f000000 : 100 + (int) k ;
		for (int mode = 0 ; mode < 3 ; mode++)
		{	wavpack_test_fixture (&io, &config, samples, 16, mode ? -1 : 16, NULL) ;
			io.sequential = mode != 0 ;
			vio = wavpack_test_vio ;
			if (mode == 2)
			{	vio.seek = NULL ;
				vio.tell = NULL ;
				} ;
			for (int type = 0 ; type < 4 ; type++)
			{	io.offset = 0 ;
				memset (&info, 0, sizeof (info)) ;
				file = sf_open_virtual (&vio, SFM_READ, &info, &io) ;
				assert (file != NULL && info.seekable == (mode == 0)) ;
				memset (&data_in, 0x55, 24 * sizeof (double)) ;
				switch (type)
				{	case 0 : count = sf_read_short (file, data_in.s, 24) ; item_size = sizeof (short) ; break ;
					case 1 : count = sf_read_int (file, data_in.i, 24) ; item_size = sizeof (int) ; break ;
					case 2 : count = sf_read_float (file, data_in.f, 24) ; item_size = sizeof (float) ; break ;
					default : count = sf_read_double (file, data_in.d, 24) ; item_size = sizeof (double) ; break ;
					} ;
				assert (count == 16 && sf_error (file) == SF_ERR_NO_ERROR) ;
				for (k = (unsigned) count * item_size ; k < 24 * item_size ; k++)
					assert (((unsigned char *) &data_in) [k] == 0) ;
				data_in.s [0] = 123 ;
				assert (sf_read_short (file, data_in.s, 1) == 0 && data_in.s [0] == 0) ;
				assert (sf_close (file) == 0) ;
				} ;
			} ;
		} ;

	config.float_norm_exp = 0 ;
	config.bits_per_sample = 16 ;
	config.bytes_per_sample = 2 ;
	for (k = 0 ; k < ARRAY_LEN (samples) ; k++)
		samples [k] = (int) k * 271 - 6000 ;
	/* Damage a later block so opening the valid first block still succeeds. */
	for (int damage = 0 ; damage < 2 ; damage++)
	{	wavpack_test_fixture (&io, &config, samples, 48, 48, NULL) ;
		first_block = 8 + io.data [4] + ((unsigned) io.data [5] << 8) + ((unsigned) io.data [6] << 16) + ((unsigned) io.data [7] << 24) ;
		second_block = 8 + io.data [first_block + 4] + ((unsigned) io.data [first_block + 5] << 8) + ((unsigned) io.data [first_block + 6] << 16) + ((unsigned) io.data [first_block + 7] << 24) ;
		assert (second_block > 33 && first_block + second_block < io.length) ;
		if (damage == 0)
			io.length = first_block + 33 ;
		else
			io.data [first_block + second_block - 1] ^= 1 ;
		memset (&info, 0, sizeof (info)) ;
		file = sf_open_virtual (&wavpack_test_vio, SFM_READ, &info, &io) ;
		assert (file != NULL) ;
		memset (data_in.s, 0x55, 64 * sizeof (short)) ;
		count = sf_read_short (file, data_in.s, 64) ;
		assert (count <= 48 && sf_error (file) == SF_ERR_MALFORMED_FILE) ;
		for (k = (unsigned) count ; k < 64 ; k++)
			assert (data_in.s [k] == 0) ;
		assert (sf_close (file) == 0) ;
		} ;

	wavpack_test_fixture (&io, &config, samples, 48, 48, NULL) ;
	memset (&info, 0, sizeof (info)) ;
	file = sf_open_virtual (&wavpack_test_vio, SFM_READ, &info, &io) ;
	assert (file != NULL) ;
	io.fail_seek = 1 ;
	assert (sf_seek (file, 32, SEEK_SET) == -1 && sf_error (file) != SF_ERR_NO_ERROR) ;
	assert (sf_read_short (file, data_in.s, 16) == 0) ;
	assert (sf_close (file) == 0) ;
	puts ("ok") ;
} /* wavpack_stream_test */

static void
wavpack_writer_test (void)
{	static WAVPACK_TEST_IO io ;
	static short samples [3 * 1024], decoded [5 * 1024] ;
	const int channels [] = { 32, 1024 } ;
	SNDFILE *file ;
	SF_INFO info ;
	sf_count_t count ;
	unsigned k ;

	print_test_name ("wavpack_writer_test", "memory") ;
	for (k = 0 ; k < ARRAY_LEN (samples) ; k++)
		samples [k] = (short) ((int) k % 127 - 63) ;
	for (k = 0 ; k < ARRAY_LEN (channels) ; k++)
	{	memset (&io, 0, sizeof (io)) ;
		memset (&info, 0, sizeof (info)) ;
		info.format = SF_FORMAT_WAVPACK | SF_FORMAT_PCM_16 ;
		info.samplerate = SAMPLE_RATE ;
		info.channels = channels [k] ;
		info.frames = 1 ; /* Writing must ignore a stale input frame count. */
		file = sf_open_virtual (&wavpack_test_vio, SFM_WRITE, &info, &io) ;
		assert (file != NULL) ;
		assert (sf_writef_short (file, samples, 3) == 3) ;
		assert (sf_write_short (file, samples, 1) == 0 && sf_error (file) != SF_ERR_NO_ERROR) ;
		assert (sf_close (file) == 0) ; /* An API rejection must not discard buffered samples. */
		io.offset = 0 ;
		memset (&info, 0, sizeof (info)) ;
		file = sf_open_virtual (&wavpack_test_vio, SFM_READ, &info, &io) ;
		assert (file != NULL && info.channels == channels [k] && info.frames == 3) ;
		memset (decoded, 0x55, sizeof (decoded)) ;
		assert (sf_readf_short (file, decoded, 5) == 3) ;
		assert (memcmp (samples, decoded, 3 * (size_t) info.channels * sizeof (short)) == 0) ;
		for (int n = 3 * info.channels ; n < 5 * info.channels ; n++)
			assert (decoded [n] == 0) ;
		assert (sf_close (file) == 0) ;
		} ;

	memset (&io, 0, sizeof (io)) ;
	io.sequential = 1 ;
	memset (&info, 0, sizeof (info)) ;
	info.format = SF_FORMAT_WAVPACK | SF_FORMAT_PCM_16 ;
	info.samplerate = SAMPLE_RATE ;
	info.channels = 1 ;
	file = sf_open_virtual (&wavpack_test_vio, SFM_WRITE, &info, &io) ;
	assert (file != NULL) ;
	assert (sf_write_short (file, samples, 16) == 16 && sf_close (file) == 0) ;
	io.offset = 0 ;
	memset (&info, 0, sizeof (info)) ;
	file = sf_open_virtual (&wavpack_test_vio, SFM_READ, &info, &io) ;
	assert (file != NULL && !info.seekable) ;
	assert (sf_read_short (file, decoded, 20) == 16 && sf_error (file) == SF_ERR_NO_ERROR) ;
	assert (memcmp (samples, decoded, 16 * sizeof (short)) == 0) ;
	assert (sf_close (file) == 0) ;

	/* Small writes fail while flushing at close; large ones fail during writing. */
	for (int deferred = 0 ; deferred < 2 ; deferred++)
	{	memset (&io, 0, sizeof (io)) ;
		memset (&info, 0, sizeof (info)) ;
		info.format = SF_FORMAT_WAVPACK | SF_FORMAT_PCM_16 ;
		info.samplerate = SAMPLE_RATE ;
		info.channels = 1 ;
		io.short_write = 1 ;
		file = sf_open_virtual (&wavpack_test_vio, SFM_WRITE, &info, &io) ;
		assert (file != NULL) ;
		memset (data_out.s, 0, 65536 * sizeof (short)) ;
		count = sf_write_short (file, data_out.s, deferred ? 16 : 65536) ;
		if (deferred)
			assert (count == 16 && sf_error (file) == SF_ERR_NO_ERROR) ;
		else
			assert (count < 65536 && sf_error (file) == SF_ERR_SYSTEM) ;
		assert (sf_close (file) == SF_ERR_SYSTEM) ;
		} ;
	puts ("ok") ;
} /* wavpack_writer_test */

static void
wavpack_external_test (void)
{	static WAVPACK_TEST_IO io ;
	const char *const tags [] = { "unknown", "ignore me", "ba\xff\xfe" "d", "ignore me", "Artist", "artist", NULL } ;
	WavpackConfig config = { 0 } ;
	int32_t samples [16] = { -1600, 0, 1600 } ;
	float floats [3] = { 1.0f, -0.5f, 0.25f }, decoded [3] ;
	SNDFILE *file ;
	SF_INFO info ;
	int channel_map [4] ;

	print_test_name ("wavpack_external_test", "memory") ;
	config.num_channels = 1 ;
	config.sample_rate = SAMPLE_RATE ;
	config.block_samples = 16 ;
	for (int bytes = 2 ; bytes <= 3 ; bytes++)
	{	config.bytes_per_sample = bytes ;
		config.bits_per_sample = bytes * 8 - 4 ;
		config.qmode = QMODE_UNSIGNED_WORDS ;
		wavpack_test_fixture (&io, &config, samples, 3, 3, tags) ;
		memset (&info, 0, sizeof (info)) ;
		file = sf_open_virtual (&wavpack_test_vio, SFM_READ, &info, &io) ;
		assert (file != NULL && info.format == (SF_FORMAT_WAVPACK | (bytes == 2 ? SF_FORMAT_PCM_16 : SF_FORMAT_PCM_24))) ;
		assert (sf_get_string (file, SF_STR_TITLE) == NULL) ;
		assert (sf_get_string (file, SF_STR_ARTIST) && !strcmp (sf_get_string (file, SF_STR_ARTIST), "artist")) ;
		assert (sf_read_int (file, data_in.i, 3) == 3) ;
		assert (data_in.i [0] == -1600 * (bytes == 2 ? 65536 : 256) && data_in.i [1] == 0 && data_in.i [2] == -data_in.i [0]) ;
		assert (sf_close (file) == 0) ;
		} ;

	config.qmode = 0 ;
	config.bits_per_sample = 32 ;
	config.bytes_per_sample = 4 ;
	config.float_norm_exp = 128 ;
	memcpy (samples, floats, sizeof (floats)) ;
	wavpack_test_fixture (&io, &config, samples, 3, 3, NULL) ;
	memset (&info, 0, sizeof (info)) ;
	file = sf_open_virtual (&wavpack_test_vio, SFM_READ, &info, &io) ;
	assert (file != NULL && info.format == (SF_FORMAT_WAVPACK | SF_FORMAT_FLOAT)) ;
	assert (sf_read_float (file, decoded, 3) == 3) ;
	for (int k = 0 ; k < 3 ; k++)
		assert (decoded [k] == floats [k] * 0.5f) ;
	assert (sf_close (file) == 0) ;

	config.float_norm_exp = 0 ;
	config.bits_per_sample = 16 ;
	config.bytes_per_sample = 2 ;
	config.num_channels = 4 ;
	config.channel_mask = 3 | (1 << 20) ; /* Only L/R are known, the reserved bit is ignored. */
	memset (samples, 0, sizeof (samples)) ;
	wavpack_test_fixture (&io, &config, samples, 3, 3, NULL) ;
	memset (&info, 0, sizeof (info)) ;
	file = sf_open_virtual (&wavpack_test_vio, SFM_READ, &info, &io) ;
	assert (file != NULL) ;
	assert (sf_command (file, SFC_GET_CHANNEL_MAP_INFO, channel_map, sizeof (channel_map)) == SF_TRUE) ;
	assert (channel_map [0] == SF_CHANNEL_MAP_LEFT && channel_map [1] == SF_CHANNEL_MAP_RIGHT) ;
	assert (channel_map [2] == SF_CHANNEL_MAP_INVALID && channel_map [3] == SF_CHANNEL_MAP_INVALID) ;
	assert (sf_close (file) == 0) ;

	config.num_channels = 1 ;
	config.channel_mask = 0 ;
	wavpack_test_fixture (&io, &config, samples, 3, 3, NULL) ;
	io.reported_length = SF_COUNT_MAX / 2 ; /* Multiplying this by sample rate overflows int64_t. */
	memset (&info, 0, sizeof (info)) ;
	file = sf_open_virtual (&wavpack_test_vio, SFM_READ, &info, &io) ;
	assert (file != NULL && info.frames == 3 && sf_current_byterate (file) == -1) ;
	assert (sf_close (file) == 0) ;
	puts ("ok") ;
} /* wavpack_external_test */

#endif

int
main (void)
{	static int otype_list [] = { SF_FORMAT_PCM_S8, SF_FORMAT_PCM_U8, SF_FORMAT_PCM_16, SF_FORMAT_PCM_24, SF_FORMAT_PCM_32, SF_FORMAT_FLOAT } ;
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
		puts ("    No WavPack tests because WavPack support was not compiled in.") ;

#if HAVE_WAVPACK
	wavpack_stream_test () ;
	wavpack_writer_test () ;
	wavpack_external_test () ;
#endif

	return 0 ;
} /* main */
