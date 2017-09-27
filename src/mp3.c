/*
** Copyright (C) 2002-2011 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include	"sfconfig.h"

#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<ctype.h>
#include	<mpg123.h>
#include	<lame/lame.h>

#include	"sndfile.h"
#include	"sfendian.h"
#include	"common.h"

/*------------------------------------------------------------------------------
** Macros to handle big/little endian issues.
*/

/*------------------------------------------------------------------------------
** Typedefs.
*/

typedef struct
{
	lame_global_flags * gfp ;
	unsigned char * encbuffer ;
} MP3_WRITE_PRIVATE ;

/*------------------------------------------------------------------------------
** Private static functions.
*/
static	int		mp3_read_open		(SF_PRIVATE * psf) ;
static	int		mp3_read_close		(SF_PRIVATE * psf) ;
static	int		mp3_read_header		(SF_PRIVATE * psf, mpg123_handle * decoder) ;
static	sf_count_t	mp3_read_2s		(SF_PRIVATE * psf, short * ptr, sf_count_t len) ;
static	sf_count_t	mp3_read_2i		(SF_PRIVATE * psf, int * ptr, sf_count_t len) ;
static	sf_count_t	mp3_read_2f		(SF_PRIVATE * psf, float * ptr, sf_count_t len) ;
static	sf_count_t	mp3_read_2d		(SF_PRIVATE * psf, double * ptr, sf_count_t len) ;
static	sf_count_t	mp3_read_seek		(SF_PRIVATE *psf, int whence, sf_count_t offset) ;
static	ssize_t		mp3_read_sf_handle	(void * handle, void * buffer, size_t bytes) ;
static	off_t		mp3_seek_sf_handle	(void * handle, off_t offset, int whence) ;
static	int		mp3_format_to_encoding	(int encoding) ;

static	int		mp3_write_open		(SF_PRIVATE * psf) ;
static	int		mp3_write_close		(SF_PRIVATE * psf) ;
static	sf_count_t	mp3_write_2s		(SF_PRIVATE *psf, const short *ptr, sf_count_t len) ;

// FIXME: This initialisation should have a better hook
static int mpg123_initialised = 0 ;

/*------------------------------------------------------------------------------
** Public function.
*/

int
mp3_open (SF_PRIVATE * psf)
{	psf->codec_data = NULL ;
	switch (psf->file.mode) {
	case (SFM_READ) :
		return mp3_read_open (psf) ;
	case (SFM_WRITE) :
		return mp3_write_open (psf) ;
	default:
		return SFE_UNIMPLEMENTED ;
	}
}

/*------------------------------------------------------------------------------
*/

// FIXME: use mpg123 error string reporting
static int
mp3_read_open (SF_PRIVATE * psf)
{
	int decoder_err = MPG123_OK ;
	mpg123_handle * decoder ;
	if (mpg123_initialised == 0)
	{	int decoder_init_err = mpg123_init () ;
		if (decoder_init_err != MPG123_OK)
		{	psf_log_printf (psf, "Failed to init mpg123.\n") ;
			return SFE_UNIMPLEMENTED ; // FIXME: semantically wrong return code
		}
	}
	mpg123_initialised++ ;

	decoder = mpg123_new (NULL, &decoder_err) ;

	if (decoder_err == MPG123_OK)
		decoder_err = mpg123_replace_reader_handle (
			decoder, mp3_read_sf_handle, mp3_seek_sf_handle, NULL) ;
	psf->fileoffset = 0 ; // FIXME: Remove this once the seek fixes are in
	if (decoder_err == MPG123_OK)
		decoder_err = mpg123_open_handle (decoder, psf) ;

	if (decoder_err == MPG123_OK)
		decoder_err = mp3_read_header (psf, decoder) ;

	if (decoder_err != MPG123_OK)
	{	psf_log_printf (psf, "Failed to initialise mp3 decoder.\n") ;
		return SFE_UNIMPLEMENTED ; // FIXME: semantically wrong return code
	}

	psf->dataoffset = psf_ftell (psf) ;
	psf->datalength = psf->filelength - psf->dataoffset ;
	psf->codec_data = decoder ;

	psf->container_close = mp3_read_close ;
	psf->seek = mp3_read_seek ;

	psf->read_short = mp3_read_2s ;
	psf->read_int = mp3_read_2i ;
	psf->read_float = mp3_read_2f ;
	psf->read_double = mp3_read_2d ;

	return 0 ;
}

static int
mp3_read_close (SF_PRIVATE * psf)
{	mpg123_handle * decoder = psf->codec_data ;
	if (decoder != NULL)
	{	// mpg123_close(decoder); <- Not sure if we need this?
		mpg123_delete (decoder) ;
		// The psf sometimes gets reused:
		psf->codec_data = NULL ;
	}
	if (!--mpg123_initialised)
		mpg123_exit () ;
	return 0 ;
}

static sf_count_t
mp3_read_seek (SF_PRIVATE *psf, int whence, sf_count_t offset)
{	mpg123_handle * decoder = psf->codec_data ;
	return mpg123_seek (decoder, whence, offset) ;
}

static int
mp3_format_to_encoding (int encoding)
{	// FIXME: This function isn't done.
	// https://www.mpg123.de/api/fmt123_8h_source.shtml
	// http://www.mega-nerd.com/libsndfile/api.html
	encoding++ ;// FIXME: this is just to suppress warning
	return SF_FORMAT_MP3 | SF_FORMAT_PCM_16 ;
}

static int
mp3_read_header (SF_PRIVATE * psf, mpg123_handle * decoder)
{	int decoder_err, channels, encoding ;
	long sample_rate ;
	decoder_err = mpg123_getformat (decoder, &sample_rate, &channels, &encoding) ;
	if (decoder_err == MPG123_OK)
	{	psf->sf.format = mp3_format_to_encoding (encoding) ;
		psf->sf.channels = channels ;
		psf->sf.samplerate = sample_rate ;
	}
	psf->sf.frames = mpg123_length (decoder) ;
	return decoder_err ;
}

static ssize_t mp3_read_sf_handle (void * handle, void * buffer, size_t bytes)
{	SF_PRIVATE * psf = handle ;
	return psf_fread (buffer, 1, bytes, psf) ;
}

static off_t mp3_seek_sf_handle (void * handle, off_t offset, int whence)
{	SF_PRIVATE * psf = handle ;
	return psf_fseek (psf, offset, whence) ;
}

static sf_count_t
mp3_read_as (SF_PRIVATE *psf, unsigned char * buffer, int encoding, size_t elem_size, sf_count_t len)
{
	size_t n_decoded = 0 ;
	mpg123_handle * decoder = psf->codec_data ;
	int decoder_err = mpg123_format (decoder, psf->sf.samplerate, psf->sf.channels, encoding) ;
	if (decoder_err == MPG123_OK)
	{	decoder_err = mpg123_read (
			decoder,
			(unsigned char *) buffer, len * elem_size,
			&n_decoded) ;
		if (decoder_err != MPG123_OK)
			psf_log_printf (psf, "Errors occured during mpg123_read.") ;
	}
	else
		psf_log_printf (psf, "Failed to set mpg123_format.\n") ;
	return psf->sf.channels * n_decoded / elem_size ;
}

static sf_count_t
mp3_read_2s	(SF_PRIVATE *psf, short *ptr, sf_count_t len)
{	return mp3_read_as (psf, (unsigned char *) ptr, MPG123_ENC_SIGNED_16, sizeof (short), len) ;
}

static sf_count_t
mp3_read_2i	(SF_PRIVATE * psf, int * ptr, sf_count_t len)
{	return mp3_read_as (psf, (unsigned char *) ptr, MPG123_ENC_SIGNED_32, sizeof (int), len) ;
}

static sf_count_t
mp3_read_2f	(SF_PRIVATE * psf, float * ptr, sf_count_t len)
{	return mp3_read_as (psf, (unsigned char *) ptr, MPG123_ENC_FLOAT_32, sizeof (float), len) ;
}

static sf_count_t
mp3_read_2d	(SF_PRIVATE * psf, double * ptr, sf_count_t len)
{	return mp3_read_as (psf, (unsigned char *) ptr, MPG123_ENC_FLOAT_64, sizeof (double), len) ;
}

#define MAX_SAMPLES_ENCODED 1024
#define ENC_BUFFER_SIZE (1.25 * MAX_SAMPLES_ENCODED) + 7200

static int
mp3_write_open (SF_PRIVATE * psf)
{	lame_global_flags * gfp ;
	MP3_WRITE_PRIVATE * codec_data ;
	int lame_err = 0 ;
	gfp = lame_init () ;
	lame_err = lame_init_params (gfp) ;
	if (lame_err)
		// FIXME: wrong return code
		return SFE_UNIMPLEMENTED ;
	psf->container_close = mp3_write_close ;
	codec_data = malloc (sizeof (MP3_WRITE_PRIVATE)) ;
	codec_data->gfp = gfp ;
	codec_data->encbuffer = malloc (ENC_BUFFER_SIZE) ;
	psf->codec_data = codec_data ;

	psf->write_short = mp3_write_2s ;
	return SFE_UNIMPLEMENTED ;
}

static int
mp3_write_close (SF_PRIVATE * psf)
{	MP3_WRITE_PRIVATE * p = psf->codec_data ;
	if (p != NULL)
	{	lame_close (p->gfp) ;
		free (p->encbuffer) ;
		free (p) ;
		psf->codec_data = NULL ;
	}
	return 0 ;
}

static sf_count_t
mp3_write_2s (SF_PRIVATE *psf, const short *ptr, sf_count_t len)
{	MP3_WRITE_PRIVATE * p = psf->codec_data ;
	sf_count_t samples_written = 0 ;
	sf_count_t written_to_file ;
	int n_bytes_encoded ;
	while (samples_written < len)
	{	sf_count_t const encoded_this_loop =
			(len > MAX_SAMPLES_ENCODED) ? MAX_SAMPLES_ENCODED : len ;
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wcast-qual"
		n_bytes_encoded = lame_encode_buffer_interleaved (
			p->gfp, (short *) ptr, encoded_this_loop, p->encbuffer, ENC_BUFFER_SIZE) ;
		#pragma GCC diagnostic pop
		written_to_file = psf_fwrite (p->encbuffer, 1, n_bytes_encoded, psf) ;
		if (written_to_file != n_bytes_encoded)
			// FIXME: error better
			return 0 ;
		samples_written += encoded_this_loop ;
	}
	return samples_written ;
}
