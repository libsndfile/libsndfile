/*
** Copyright (C) 2002-2005 Erik de Castro Lopo <erikd@mega-nerd.com>
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

/*
**	This is the OKI / Dialogic ADPCM encoder/decoder. It converts from
**	12 bit linear sample data to a 4 bit ADPCM.
*/

/*
 * Note: some early Dialogic hardware does not always reset the ADPCM encoder
 * at the start of each vox file. This can result in clipping and/or DC offset
 * problems when it comes to decoding the audio. Whilst little can be done
 * about the clipping, a DC offset can be removed by passing the decoded audio
 * through a high-pass filter at e.g. 10Hz.
 */

#include	"sfconfig.h"

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"sndfile.h"
#include	"sfendian.h"
#include	"float_cast.h"
#include	"common.h"
#include	"ima_oki_adpcm.h"

#define		VOX_DATA_LEN	2048
#define		PCM_DATA_LEN	(VOX_DATA_LEN *2)

typedef struct
{
	IMA_OKI_ADPCM codec ;
	int		vox_bytes, pcm_samples ;

	unsigned char	vox_data [VOX_DATA_LEN] ;
	short 			pcm_data [PCM_DATA_LEN] ;
} VOX_ADPCM_PRIVATE ;

static int vox_adpcm_encode_block (VOX_ADPCM_PRIVATE *pvox) ;
static int vox_adpcm_decode_block (VOX_ADPCM_PRIVATE *pvox) ;

static sf_count_t vox_read_s (SF_PRIVATE *psf, short *ptr, sf_count_t len) ;
static sf_count_t vox_read_i (SF_PRIVATE *psf, int *ptr, sf_count_t len) ;
static sf_count_t vox_read_f (SF_PRIVATE *psf, float *ptr, sf_count_t len) ;
static sf_count_t vox_read_d (SF_PRIVATE *psf, double *ptr, sf_count_t len) ;

static sf_count_t vox_write_s (SF_PRIVATE *psf, const short *ptr, sf_count_t len) ;
static sf_count_t vox_write_i (SF_PRIVATE *psf, const int *ptr, sf_count_t len) ;
static sf_count_t vox_write_f (SF_PRIVATE *psf, const float *ptr, sf_count_t len) ;
static sf_count_t vox_write_d (SF_PRIVATE *psf, const double *ptr, sf_count_t len) ;

static int vox_read_block (SF_PRIVATE *psf, VOX_ADPCM_PRIVATE *pvox, short *ptr, int len) ;

/*------------------------------------------------------------------------------
*/

static int
codec_close (SF_PRIVATE * psf)
{
	VOX_ADPCM_PRIVATE * p = (VOX_ADPCM_PRIVATE *) psf->codec_data ;

	if (p->codec.errors)
		psf_log_printf (psf, "*** Warning : ADPCM state errors: %d\n", p->codec.errors) ;
	return p->codec.errors ;
} /* code_close */

int
vox_adpcm_init (SF_PRIVATE *psf)
{	VOX_ADPCM_PRIVATE *pvox = NULL ;

	if (psf->mode == SFM_RDWR)
		return SFE_BAD_MODE_RW ;

	if (psf->mode == SFM_WRITE && psf->sf.channels != 1)
		return SFE_CHANNEL_COUNT ;

	if ((pvox = malloc (sizeof (VOX_ADPCM_PRIVATE))) == NULL)
		return SFE_MALLOC_FAILED ;

	psf->codec_data = (void*) pvox ;
	memset (pvox, 0, sizeof (VOX_ADPCM_PRIVATE)) ;

	if (psf->mode == SFM_WRITE)
	{	psf->write_short	= vox_write_s ;
		psf->write_int		= vox_write_i ;
		psf->write_float	= vox_write_f ;
		psf->write_double	= vox_write_d ;
		}
	else
	{	psf_log_printf (psf, "Header-less OKI Dialogic ADPCM encoded file.\n") ;
		psf_log_printf (psf, "Setting up for 8kHz, mono, Vox ADPCM.\n") ;

		psf->read_short		= vox_read_s ;
		psf->read_int		= vox_read_i ;
		psf->read_float		= vox_read_f ;
		psf->read_double	= vox_read_d ;
		} ;

	/* Standard sample rate chennels etc. */
	if (psf->sf.samplerate < 1)
		psf->sf.samplerate	= 8000 ;
	psf->sf.channels	= 1 ;

	psf->sf.frames = psf->filelength * 2 ;

	psf->sf.seekable = SF_FALSE ;
	psf->codec_close = codec_close ;

	/* Seek back to start of data. */
	if (psf_fseek (psf, 0 , SEEK_SET) == -1)
		return SFE_BAD_SEEK ;

	adpcm_init (&pvox->codec, ADPCM_OKI) ;

	return 0 ;
} /* vox_adpcm_init */

/*------------------------------------------------------------------------------
*/

static int
vox_adpcm_encode_block (VOX_ADPCM_PRIVATE *pvox)
{	unsigned char code ;
	int j, k ;

	/* If data_count is odd, add an extra zero valued sample. */
	if (pvox->pcm_samples & 1)
		pvox->pcm_data [pvox->pcm_samples++] = 0 ;

	for (j = k = 0 ; k < pvox->pcm_samples ; j++)
	{	code = adpcm_encode (&pvox->codec, pvox->pcm_data [k++]) << 4 ;
		code |= adpcm_encode (&pvox->codec, pvox->pcm_data [k++]) ;
		pvox->vox_data [j] = code ;
		} ;

	pvox->vox_bytes = j ;

	return 0 ;
} /* vox_adpcm_encode_block */

static int
vox_adpcm_decode_block (VOX_ADPCM_PRIVATE *pvox)
{	unsigned char code ;
	int j, k ;

	for (j = k = 0 ; j < pvox->vox_bytes ; j++)
	{	code = pvox->vox_data [j] ;
		pvox->pcm_data [k++] = adpcm_decode (&pvox->codec, code >> 4) ;
		pvox->pcm_data [k++] = adpcm_decode (&pvox->codec, code) ;
		} ;

	pvox->pcm_samples = k ;

	return 0 ;
} /* vox_adpcm_decode_block */

/*==============================================================================
*/

static int
vox_read_block (SF_PRIVATE *psf, VOX_ADPCM_PRIVATE *pvox, short *ptr, int len)
{	int	indx = 0, k ;

	while (indx < len)
	{	pvox->vox_bytes = (len - indx > PCM_DATA_LEN) ? VOX_DATA_LEN : (len - indx + 1) / 2 ;

		if ((k = psf_fread (pvox->vox_data, 1, pvox->vox_bytes, psf)) != pvox->vox_bytes)
		{	if (psf_ftell (psf) != psf->filelength)
				psf_log_printf (psf, "*** Warning : short read (%d != %d).\n", k, pvox->vox_bytes) ;
			if (k == 0)
				break ;
			} ;

		pvox->vox_bytes = k ;

		vox_adpcm_decode_block (pvox) ;

		memcpy (&(ptr [indx]), pvox->pcm_data, pvox->pcm_samples * sizeof (short)) ;
		indx += pvox->pcm_samples ;
		} ;

	return indx ;
} /* vox_read_block */


static sf_count_t
vox_read_s (SF_PRIVATE *psf, short *ptr, sf_count_t len)
{	VOX_ADPCM_PRIVATE 	*pvox ;
	int			readcount, count ;
	sf_count_t	total = 0 ;

	if (! psf->codec_data)
		return 0 ;
	pvox = (VOX_ADPCM_PRIVATE*) psf->codec_data ;

	while (len > 0)
	{	readcount = (len > 0x10000000) ? 0x10000000 : (int) len ;

		count = vox_read_block (psf, pvox, ptr, readcount) ;

		total += count ;
		len -= count ;
		if (count != readcount)
			break ;
		} ;

	return total ;
} /* vox_read_s */

static sf_count_t
vox_read_i	(SF_PRIVATE *psf, int *ptr, sf_count_t len)
{	VOX_ADPCM_PRIVATE *pvox ;
	short		*sptr ;
	int			k, bufferlen, readcount, count ;
	sf_count_t	total = 0 ;

	if (! psf->codec_data)
		return 0 ;
	pvox = (VOX_ADPCM_PRIVATE*) psf->codec_data ;

	sptr = psf->u.sbuf ;
	bufferlen = ARRAY_LEN (psf->u.sbuf) ;
	while (len > 0)
	{	readcount = (len >= bufferlen) ? bufferlen : (int) len ;
		count = vox_read_block (psf, pvox, sptr, readcount) ;
		for (k = 0 ; k < readcount ; k++)
			ptr [total + k] = ((int) sptr [k]) << 16 ;
		total += count ;
		len -= readcount ;
		if (count != readcount)
			break ;
		} ;

	return total ;
} /* vox_read_i */

static sf_count_t
vox_read_f (SF_PRIVATE *psf, float *ptr, sf_count_t len)
{	VOX_ADPCM_PRIVATE *pvox ;
	short		*sptr ;
	int			k, bufferlen, readcount, count ;
	sf_count_t	total = 0 ;
	float		normfact ;

	if (! psf->codec_data)
		return 0 ;
	pvox = (VOX_ADPCM_PRIVATE*) psf->codec_data ;

	normfact = (psf->norm_float == SF_TRUE) ? 1.0 / ((float) 0x8000) : 1.0 ;

	sptr = psf->u.sbuf ;
	bufferlen = ARRAY_LEN (psf->u.sbuf) ;
	while (len > 0)
	{	readcount = (len >= bufferlen) ? bufferlen : (int) len ;
		count = vox_read_block (psf, pvox, sptr, readcount) ;
		for (k = 0 ; k < readcount ; k++)
			ptr [total + k] = normfact * (float) (sptr [k]) ;
		total += count ;
		len -= readcount ;
		if (count != readcount)
			break ;
		} ;

	return total ;
} /* vox_read_f */

static sf_count_t
vox_read_d (SF_PRIVATE *psf, double *ptr, sf_count_t len)
{	VOX_ADPCM_PRIVATE *pvox ;
	short		*sptr ;
	int			k, bufferlen, readcount, count ;
	sf_count_t	total = 0 ;
	double 		normfact ;

	if (! psf->codec_data)
		return 0 ;
	pvox = (VOX_ADPCM_PRIVATE*) psf->codec_data ;

	normfact = (psf->norm_double == SF_TRUE) ? 1.0 / ((double) 0x8000) : 1.0 ;

	sptr = psf->u.sbuf ;
	bufferlen = ARRAY_LEN (psf->u.sbuf) ;
	while (len > 0)
	{	readcount = (len >= bufferlen) ? bufferlen : (int) len ;
		count = vox_read_block (psf, pvox, sptr, readcount) ;
		for (k = 0 ; k < readcount ; k++)
			ptr [total + k] = normfact * (double) (sptr [k]) ;
		total += count ;
		len -= readcount ;
		if (count != readcount)
			break ;
		} ;

	return total ;
} /* vox_read_d */

/*------------------------------------------------------------------------------
*/

static int
vox_write_block (SF_PRIVATE *psf, VOX_ADPCM_PRIVATE *pvox, const short *ptr, int len)
{	int	indx = 0, k ;

	while (indx < len)
	{	pvox->pcm_samples = (len - indx > PCM_DATA_LEN) ? PCM_DATA_LEN : len - indx ;

		memcpy (pvox->pcm_data, &(ptr [indx]), pvox->pcm_samples * sizeof (short)) ;

		vox_adpcm_encode_block (pvox) ;

		if ((k = psf_fwrite (pvox->vox_data, 1, pvox->vox_bytes, psf)) != pvox->vox_bytes)
			psf_log_printf (psf, "*** Warning : short write (%d != %d).\n", k, pvox->vox_bytes) ;

		indx += pvox->pcm_samples ;
		} ;

	return indx ;
} /* vox_write_block */

static sf_count_t
vox_write_s (SF_PRIVATE *psf, const short *ptr, sf_count_t len)
{	VOX_ADPCM_PRIVATE 	*pvox ;
	int			writecount, count ;
	sf_count_t	total = 0 ;

	if (! psf->codec_data)
		return 0 ;
	pvox = (VOX_ADPCM_PRIVATE*) psf->codec_data ;

	while (len)
	{	writecount = (len > 0x10000000) ? 0x10000000 : (int) len ;

		count = vox_write_block (psf, pvox, ptr, writecount) ;

		total += count ;
		len -= count ;
		if (count != writecount)
			break ;
		} ;

	return total ;
} /* vox_write_s */

static sf_count_t
vox_write_i	(SF_PRIVATE *psf, const int *ptr, sf_count_t len)
{	VOX_ADPCM_PRIVATE *pvox ;
	short		*sptr ;
	int			k, bufferlen, writecount, count ;
	sf_count_t	total = 0 ;

	if (! psf->codec_data)
		return 0 ;
	pvox = (VOX_ADPCM_PRIVATE*) psf->codec_data ;

	sptr = psf->u.sbuf ;
	bufferlen = ARRAY_LEN (psf->u.sbuf) ;
	while (len > 0)
	{	writecount = (len >= bufferlen) ? bufferlen : (int) len ;
		for (k = 0 ; k < writecount ; k++)
			sptr [k] = ptr [total + k] >> 16 ;
		count = vox_write_block (psf, pvox, sptr, writecount) ;
		total += count ;
		len -= writecount ;
		if (count != writecount)
			break ;
		} ;

	return total ;
} /* vox_write_i */

static sf_count_t
vox_write_f (SF_PRIVATE *psf, const float *ptr, sf_count_t len)
{	VOX_ADPCM_PRIVATE *pvox ;
	short		*sptr ;
	int			k, bufferlen, writecount, count ;
	sf_count_t	total = 0 ;
	float		normfact ;

	if (! psf->codec_data)
		return 0 ;
	pvox = (VOX_ADPCM_PRIVATE*) psf->codec_data ;

	normfact = (psf->norm_float == SF_TRUE) ? (1.0 * 0x7FFF) : 1.0 ;

	sptr = psf->u.sbuf ;
	bufferlen = ARRAY_LEN (psf->u.sbuf) ;
	while (len > 0)
	{	writecount = (len >= bufferlen) ? bufferlen : (int) len ;
		for (k = 0 ; k < writecount ; k++)
			sptr [k] = lrintf (normfact * ptr [total + k]) ;
		count = vox_write_block (psf, pvox, sptr, writecount) ;
		total += count ;
		len -= writecount ;
		if (count != writecount)
			break ;
		} ;

	return total ;
} /* vox_write_f */

static sf_count_t
vox_write_d	(SF_PRIVATE *psf, const double *ptr, sf_count_t len)
{	VOX_ADPCM_PRIVATE *pvox ;
	short		*sptr ;
	int			k, bufferlen, writecount, count ;
	sf_count_t	total = 0 ;
	double 		normfact ;

	if (! psf->codec_data)
		return 0 ;
	pvox = (VOX_ADPCM_PRIVATE*) psf->codec_data ;

	normfact = (psf->norm_double == SF_TRUE) ? (1.0 * 0x7FFF) : 1.0 ;

	sptr = psf->u.sbuf ;
	bufferlen = ARRAY_LEN (psf->u.sbuf) ;
	while (len > 0)
	{	writecount = (len >= bufferlen) ? bufferlen : (int) len ;
		for (k = 0 ; k < writecount ; k++)
			sptr [k] = lrint (normfact * ptr [total + k]) ;
		count = vox_write_block (psf, pvox, sptr, writecount) ;
		total += count ;
		len -= writecount ;
		if (count != writecount)
			break ;
		} ;

	return total ;
} /* vox_write_d */


/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: e15e97fe-ff9d-4b46-a489-7059fb2d0b1e
*/
