/*
** Copyright (C) 1999-2011 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <sndfile.h>

#include "utils.h"

static void vio_test (const char *fname, int format) ;
static void vio_empty_chunk_test (void) ;

int
main (void)
{
	vio_test ("vio_pcm16.wav", SF_FORMAT_WAV | SF_FORMAT_PCM_16) ;
	vio_test ("vio_pcm24.aiff", SF_FORMAT_AIFF | SF_FORMAT_PCM_24) ;
	vio_test ("vio_float.au", SF_FORMAT_AU | SF_FORMAT_FLOAT) ;
	vio_test ("vio_pcm24.paf", SF_FORMAT_PAF | SF_FORMAT_PCM_24) ;
#if HAVE_WAVPACK
	vio_test ("vio_wavpack16.wv", SF_FORMAT_WAVPACK | SF_FORMAT_PCM_16) ;
	vio_test ("vio_wavpack24.wv", SF_FORMAT_WAVPACK | SF_FORMAT_PCM_24) ;
	vio_test ("vio_wavpack32.wv", SF_FORMAT_WAVPACK | SF_FORMAT_PCM_32) ;
	vio_test ("vio_wavpackf32.wv", SF_FORMAT_WAVPACK | SF_FORMAT_FLOAT) ;
#endif
	vio_empty_chunk_test () ;

	return 0 ;
} /* main */

/*==============================================================================
*/

typedef struct
{	sf_count_t offset, length ;
	unsigned char data [16 * 1024] ;
} VIO_DATA ;

static sf_count_t
vfget_filelen (void *user_data)
{	VIO_DATA *vf = (VIO_DATA *) user_data ;

	return vf->length ;
} /* vfget_filelen */

static sf_count_t
vfseek (sf_count_t offset, int whence, void *user_data)
{	VIO_DATA *vf = (VIO_DATA *) user_data ;

	switch (whence)
	{	case SEEK_SET :
			vf->offset = offset ;
			break ;

		case SEEK_CUR :
			vf->offset = vf->offset + offset ;
			break ;

		case SEEK_END :
			vf->offset = vf->length + offset ;
			break ;
		default :
			break ;
		} ;

	return vf->offset ;
} /* vfseek */

static sf_count_t
vfread (void *ptr, sf_count_t count, void *user_data)
{	VIO_DATA *vf = (VIO_DATA *) user_data ;

	/*
	**	This will break badly for files over 2Gig in length, but
	**	is sufficient for testing.
	*/
	if (vf->offset + count > vf->length)
		count = vf->length - vf->offset ;

	memcpy (ptr, vf->data + vf->offset, count) ;
	vf->offset += count ;

	return count ;
} /* vfread */

static sf_count_t
vfwrite (const void *ptr, sf_count_t count, void *user_data)
{	VIO_DATA *vf = (VIO_DATA *) user_data ;

	/*
	**	This will break badly for files over 2Gig in length, but
	**	is sufficient for testing.
	*/
	if (vf->offset >= SIGNED_SIZEOF (vf->data))
		return 0 ;

	if (vf->offset + count > SIGNED_SIZEOF (vf->data))
		count = sizeof (vf->data) - vf->offset ;

	memcpy (vf->data + vf->offset, ptr, (size_t) count) ;
	vf->offset += count ;

	if (vf->offset > vf->length)
		vf->length = vf->offset ;

	return count ;
} /* vfwrite */

static sf_count_t
vftell (void *user_data)
{	VIO_DATA *vf = (VIO_DATA *) user_data ;

	return vf->offset ;
} /* vftell */


/*==============================================================================
*/

static void
gen_short_data (short * data, int len, int start)
{	int k ;

	for (k = 0 ; k < len ; k++)
		data [k] = start + k ;
} /* gen_short_data */


static void
check_short_data (short * data, int len, int start, int line)
{	int k ;

	for (k = 0 ; k < len ; k++)
		if (data [k] != start + k)
		{	printf ("\n\nLine %d : data [%d] = %d (should be %d).\n\n", line, k, data [k], start + k) ;
			exit (1) ;
			} ;
} /* gen_short_data */

/*------------------------------------------------------------------------------
*/

static void
vio_test (const char *fname, int format)
{	static VIO_DATA vio_data ;
	static short data [256] ;

	SF_VIRTUAL_IO vio ;
	SNDFILE * file ;
	SF_INFO sfinfo ;

	print_test_name ("virtual i/o test", fname) ;

	/* Set up pointers to the locally defined functions. */
	vio.get_filelen = vfget_filelen ;
	vio.seek = vfseek ;
	vio.read = vfread ;
	vio.write = vfwrite ;
	vio.tell = vftell ;

	/* Set virtual file offset and length to zero. */
	vio_data.offset = 0 ;
	vio_data.length = 0 ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	sfinfo.format = format ;
	sfinfo.channels = 2 ;
	sfinfo.samplerate = 44100 ;

	if ((file = sf_open_virtual (&vio, SFM_WRITE, &sfinfo, &vio_data)) == NULL)
	{	printf ("\n\nLine %d : sf_open_write failed with error : ", __LINE__) ;
		fflush (stdout) ;
		puts (sf_strerror (NULL)) ;
		exit (1) ;
		} ;

	if (vfget_filelen (&vio_data) < 0)
	{	printf ("\n\nLine %d : vfget_filelen returned negative length.\n\n", __LINE__) ;
		exit (1) ;
		} ;

	gen_short_data (data, ARRAY_LEN (data), 0) ;
	sf_write_short (file, data, ARRAY_LEN (data)) ;

	gen_short_data (data, ARRAY_LEN (data), 1) ;
	sf_write_short (file, data, ARRAY_LEN (data)) ;

	gen_short_data (data, ARRAY_LEN (data), 2) ;
	sf_write_short (file, data, ARRAY_LEN (data)) ;

	sf_close (file) ;

	/* Now test read. */
	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	vio_data.offset = 0 ;

	if ((file = sf_open_virtual (&vio, SFM_READ, &sfinfo, &vio_data)) == NULL)
	{	printf ("\n\nLine %d : sf_open_write failed with error : ", __LINE__) ;
		fflush (stdout) ;
		puts (sf_strerror (NULL)) ;

		dump_data_to_file (fname, vio_data.data, (unsigned int) vio_data.length) ;
		exit (1) ;
		} ;


	sf_read_short (file, data, ARRAY_LEN (data)) ;
	check_short_data (data, ARRAY_LEN (data), 0, __LINE__) ;

	sf_read_short (file, data, ARRAY_LEN (data)) ;
	check_short_data (data, ARRAY_LEN (data), 1, __LINE__) ;

	sf_read_short (file, data, ARRAY_LEN (data)) ;
	check_short_data (data, ARRAY_LEN (data), 2, __LINE__) ;

	sf_close (file) ;

	puts ("ok") ;
} /* vio_test */


static void
vio_empty_chunk_test (void)
{	static VIO_DATA vio_data ;
	static const unsigned char aiff_data [] =
	{	'F' , 'O' , 'R' , 'M' ,
		0x00, 0x00, 0x00, 0x36, /* FORM length */
		'A' , 'I' , 'F' , 'F' ,
		'C' , 'O' , 'M' , 'M' ,
		0x00, 0x00, 0x00, 0x12, /* COMM length */
		0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x40, 0x0E, 0xAC, 0x44,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		'A' , 'N' , 'N' , 'O' ,
		0x00, 0x00, 0x00, 0x00, /* ANNO length */
		'S' , 'S' , 'N' , 'D' ,
		0x00, 0x00, 0x00, 0x08, /* SSND length */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		} ;

	SF_VIRTUAL_IO vio ;
	SNDFILE * file ;
	SF_INFO sfinfo ;
	SF_CHUNK_INFO chunk_info ;
	SF_CHUNK_ITERATOR * iterator ;
	unsigned char byte ;
	int err ;

	print_test_name ("virtual i/o test", "empty chunk") ;

	vio.get_filelen = vfget_filelen ;
	vio.seek = vfseek ;
	vio.read = vfread ;
	vio.write = vfwrite ;
	vio.tell = vftell ;

	memcpy (vio_data.data, aiff_data, sizeof (aiff_data)) ;
	vio_data.offset = 0 ;
	vio_data.length = sizeof (aiff_data) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	if ((file = sf_open_virtual (&vio, SFM_READ, &sfinfo, &vio_data)) == NULL)
	{	printf ("\n\nLine %d : sf_open_virtual failed with error : ", __LINE__) ;
		fflush (stdout) ;
		puts (sf_strerror (NULL)) ;
		exit (1) ;
		} ;

	memset (&chunk_info, 0, sizeof (chunk_info)) ;
	memcpy (chunk_info.id, "ANNO", 4) ;
	chunk_info.id_size = 4 ;

	if ((iterator = sf_get_chunk_iterator (file, &chunk_info)) == NULL)
	{	printf ("\n\nLine %d : sf_get_chunk_iterator returned NULL.\n\n", __LINE__) ;
		exit (1) ;
		} ;

	chunk_info.data = &byte ;

	if ((err = sf_get_chunk_data (iterator, &chunk_info)) != SF_ERR_NO_ERROR)
	{	printf ("\n\nLine %d : sf_get_chunk_data returned %s.\n\n", __LINE__, sf_error_number (err)) ;
		exit (1) ;
		} ;

	sf_close (file) ;

	puts ("ok") ;
} /* vio_empty_chunk_test */
