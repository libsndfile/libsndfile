/*
** Copyright (C) 2002-2014 Erik de Castro Lopo <erikd@mega-nerd.com>
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
#else
#include "sf_unistd.h"
#endif

#include <sndfile.h>

#include "utils.h"

#define	BUFFER_SIZE		(10000)

#ifndef		M_PI
#define		M_PI		3.14159265358979323846264338
#endif

static	void	dwvw_test (const char *filename, int format, int bit_width) ;
static	void	read_past_end_test (const char *filename, int format) ;

int
main (void)
{
	dwvw_test ("dwvw12.raw", SF_FORMAT_RAW | SF_FORMAT_DWVW_12, 12) ;
	dwvw_test ("dwvw16.raw", SF_FORMAT_RAW | SF_FORMAT_DWVW_16, 16) ;
	dwvw_test ("dwvw24.raw", SF_FORMAT_RAW | SF_FORMAT_DWVW_24, 24) ;

	read_past_end_test ("dwvw12.raw", SF_FORMAT_RAW | SF_FORMAT_DWVW_12) ;
	read_past_end_test ("dwvw16.raw", SF_FORMAT_RAW | SF_FORMAT_DWVW_16) ;
	read_past_end_test ("dwvw24.raw", SF_FORMAT_RAW | SF_FORMAT_DWVW_24) ;
	read_past_end_test ("vox_adpcm.raw", SF_FORMAT_RAW | SF_FORMAT_VOX_ADPCM) ;

	return 0 ;
} /* main */

static void
dwvw_test (const char *filename, int format, int bit_width)
{	static	int		write_buf [BUFFER_SIZE] ;
	static	int		read_buf [BUFFER_SIZE] ;

	SNDFILE	*file ;
	SF_INFO sfinfo ;
	double 	value ;
	int		k, bit_mask ;

	srand (123456) ;

	/* Only want to grab the top bit_width bits. */
	bit_mask = arith_shift_left (-1, 32 - bit_width) ;

	print_test_name ("dwvw_test", filename) ;

	sf_info_setup (&sfinfo, format, 44100, 1) ;

	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_TRUE, __LINE__) ;

	/* Generate random.frames. */
	for (k = 0 ; k < BUFFER_SIZE / 2 ; k++)
	{	value = 0x7FFFFFFF * sin (123.0 / sfinfo.samplerate * 2 * k * M_PI) ;
		write_buf [k] = bit_mask & lrint (value) ;
		} ;

	for ( ; k < BUFFER_SIZE ; k++)
		write_buf [k] = bit_mask & (arith_shift_left (rand (), 11) ^ (rand () >> 11)) ;

	sf_write_int (file, write_buf, BUFFER_SIZE) ;
	sf_close (file) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_TRUE, __LINE__) ;

	if ((k = (int) sf_read_int (file, read_buf, BUFFER_SIZE)) != BUFFER_SIZE)
	{	printf ("Error (line %d) : Only read %d/%d.frames.\n", __LINE__, k, BUFFER_SIZE) ;
		exit (1) ;
		}

	for (k = 0 ; k < BUFFER_SIZE ; k++)
	{	if (read_buf [k] != write_buf [k])
		{	printf ("Error (line %d) : %d != %d at position %d/%d\n", __LINE__,
				write_buf [k] >> (32 - bit_width), read_buf [k] >> (32 - bit_width),
				k, BUFFER_SIZE) ;
			oct_save_int (write_buf, read_buf, BUFFER_SIZE) ;
			exit (1) ;
			} ;
		} ;

	sf_close (file) ;

	unlink (filename) ;
	printf ("ok\n") ;

	return ;
} /* dwvw_test */

/*
**	When a read asks for more frames than the file contains, the decoder
**	produces fewer frames than requested. The short/float/double readers
**	decode into a stack buffer and then convert it; they must convert only
**	the frames the decoder actually produced, never the whole request, or
**	the uninitialised tail of the stack buffer leaks into the caller's
**	buffer past the returned frame count.
*/

#define	PE_WRITE_FRAMES		100
#define	PE_READ_FRAMES		400
#define	PE_SENTINEL			0x5a5a

static void
read_past_end_test (const char *filename, int format)
{	static short	wbuf [PE_WRITE_FRAMES] ;
	static short	sbuf [PE_READ_FRAMES] ;
	static int		ibuf [PE_READ_FRAMES] ;
	static float	fbuf [PE_READ_FRAMES] ;
	static double	dbuf [PE_READ_FRAMES] ;

	SNDFILE		*file ;
	SF_INFO		sfinfo ;
	sf_count_t	frames ;
	int			k, is_vox ;

	print_test_name ("read_past_end_test", filename) ;

	is_vox = (format & SF_FORMAT_SUBMASK) == SF_FORMAT_VOX_ADPCM ;

	sf_info_setup (&sfinfo, format, 44100, 1) ;

	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_TRUE, __LINE__) ;
	for (k = 0 ; k < PE_WRITE_FRAMES ; k++)
		wbuf [k] = (short) (k * 101) ;
	test_writef_short_or_die (file, 0, wbuf, PE_WRITE_FRAMES, __LINE__) ;
	sf_close (file) ;

	/*
	**	Exercise the conversion readers. The dwvw int reader and the vox
	**	short reader decode straight into the caller's buffer, so for each
	**	format test the integer width that goes through the stack buffer.
	*/
	if (is_vox)
	{	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_TRUE, __LINE__) ;
		for (k = 0 ; k < PE_READ_FRAMES ; k++)
			ibuf [k] = PE_SENTINEL ;
		frames = sf_readf_int (file, ibuf, PE_READ_FRAMES) ;
		for (k = (int) frames ; k < PE_READ_FRAMES ; k++)
			exit_if_true (ibuf [k] != PE_SENTINEL,
				"\n\nLine %d : int read wrote past returned frame count %d at %d.\n\n", __LINE__, (int) frames, k) ;
		sf_close (file) ;
		}
	else
	{	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_TRUE, __LINE__) ;
		for (k = 0 ; k < PE_READ_FRAMES ; k++)
			sbuf [k] = PE_SENTINEL ;
		frames = sf_readf_short (file, sbuf, PE_READ_FRAMES) ;
		for (k = (int) frames ; k < PE_READ_FRAMES ; k++)
			exit_if_true (sbuf [k] != PE_SENTINEL,
				"\n\nLine %d : short read wrote past returned frame count %d at %d.\n\n", __LINE__, (int) frames, k) ;
		sf_close (file) ;
		} ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_TRUE, __LINE__) ;
	for (k = 0 ; k < PE_READ_FRAMES ; k++)
		fbuf [k] = PE_SENTINEL ;
	frames = sf_readf_float (file, fbuf, PE_READ_FRAMES) ;
	for (k = (int) frames ; k < PE_READ_FRAMES ; k++)
		exit_if_true (fbuf [k] != (float) PE_SENTINEL,
			"\n\nLine %d : float read wrote past returned frame count %d at %d.\n\n", __LINE__, (int) frames, k) ;
	sf_close (file) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_TRUE, __LINE__) ;
	for (k = 0 ; k < PE_READ_FRAMES ; k++)
		dbuf [k] = PE_SENTINEL ;
	frames = sf_readf_double (file, dbuf, PE_READ_FRAMES) ;
	for (k = (int) frames ; k < PE_READ_FRAMES ; k++)
		exit_if_true (dbuf [k] != (double) PE_SENTINEL,
			"\n\nLine %d : double read wrote past returned frame count %d at %d.\n\n", __LINE__, (int) frames, k) ;
	sf_close (file) ;

	unlink (filename) ;
	printf ("ok\n") ;

	return ;
} /* read_past_end_test */

