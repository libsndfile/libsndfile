/*
** Copyright (C) 1999-2005 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include "sfconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sndfile.h"
#include "sfendian.h"
#include "float_cast.h"
#include "common.h"
#include "au.h"
#include "G72x/g72x.h"

static	int	au_g72x_decode_block	(SF_PRIVATE *psf, G72x_DATA *pg72x) ;
static	int	au_g72x_encode_block	(SF_PRIVATE *psf, G72x_DATA *pg72x) ;

static	sf_count_t	au_g72x_read_s	(SF_PRIVATE *psf, short *ptr, sf_count_t len) ;
static	sf_count_t	au_g72x_read_i	(SF_PRIVATE *psf, int *ptr, sf_count_t len) ;
static	sf_count_t	au_g72x_read_f	(SF_PRIVATE *psf, float *ptr, sf_count_t len) ;
static	sf_count_t	au_g72x_read_d	(SF_PRIVATE *psf, double *ptr, sf_count_t len) ;

static	sf_count_t	au_g72x_write_s	(SF_PRIVATE *psf, const short *ptr, sf_count_t len) ;
static	sf_count_t	au_g72x_write_i	(SF_PRIVATE *psf, const int *ptr, sf_count_t len) ;
static	sf_count_t	au_g72x_write_f	(SF_PRIVATE *psf, const float *ptr, sf_count_t len) ;
static	sf_count_t	au_g72x_write_d	(SF_PRIVATE *psf, const double *ptr, sf_count_t len) ;

static	sf_count_t au_g72x_seek	(SF_PRIVATE *psf, int mode, sf_count_t offset) ;

static	int	au_g72x_close	(SF_PRIVATE *psf) ;


/*============================================================================================
** WAV G721 Reader initialisation function.
*/

int
au_g72x_reader_init	(SF_PRIVATE *psf, int codec)
{	G72x_DATA	*pg72x ;
	int	bitspersample ;

	psf->sf.seekable = SF_FALSE ;

	if (psf->sf.channels != 1)
		return SFE_G72X_NOT_MONO ;

	if (! (pg72x = malloc (sizeof (G72x_DATA))))
		return SFE_MALLOC_FAILED ;

	psf->fdata = (void*) pg72x ;

	pg72x->blockcount = 0 ;
	pg72x->samplecount = 0 ;

	switch (codec)
	{	case AU_H_G721_32 :
				g72x_reader_init (pg72x, G721_32_BITS_PER_SAMPLE) ;
				pg72x->bytesperblock = G721_32_BYTES_PER_BLOCK ;
				bitspersample = G721_32_BITS_PER_SAMPLE ;
				break ;

		case AU_H_G723_24:
				g72x_reader_init (pg72x, G723_24_BITS_PER_SAMPLE) ;
				pg72x->bytesperblock = G723_24_BYTES_PER_BLOCK ;
				bitspersample = G723_24_BITS_PER_SAMPLE ;
				break ;

		case AU_H_G723_40:
				g72x_reader_init (pg72x, G723_40_BITS_PER_SAMPLE) ;
				pg72x->bytesperblock = G723_40_BYTES_PER_BLOCK ;
				bitspersample = G723_40_BITS_PER_SAMPLE ;
				break ;

		default : return SFE_UNIMPLEMENTED ;
		} ;

	psf->read_short		= au_g72x_read_s ;
	psf->read_int		= au_g72x_read_i ;
	psf->read_float		= au_g72x_read_f ;
	psf->read_double	= au_g72x_read_d ;

 	psf->seek	= au_g72x_seek ;
 	psf->close	= au_g72x_close ;

	psf->blockwidth = psf->bytewidth = 1 ;

	psf->filelength = psf_get_filelen (psf) ;
	psf->datalength = psf->filelength - psf->dataoffset ;

	if (psf->datalength % pg72x->blocksize)
		pg72x->blocks = (psf->datalength / pg72x->blocksize) + 1 ;
	else
		pg72x->blocks = psf->datalength / pg72x->blocksize ;

	psf->sf.frames = (8 * psf->datalength) / bitspersample ;

	if ((psf->sf.frames * bitspersample) / 8 != psf->datalength)
		psf_log_printf (psf, "*** Warning : weird psf->datalength.\n") ;

	au_g72x_decode_block (psf, pg72x) ;

	return 0 ;
} /* au_g72x_reader_init */

/*============================================================================================
** WAV G721 writer initialisation function.
*/

int
au_g72x_writer_init	(SF_PRIVATE *psf, int codec)
{	G72x_DATA	*pg72x ;
	int bitspersample ;

	psf->sf.seekable = SF_FALSE ;

	if (psf->sf.channels != 1)
		return SFE_G72X_NOT_MONO ;

	if (! (pg72x = malloc (sizeof (G72x_DATA))))
		return SFE_MALLOC_FAILED ;

	psf->fdata = (void*) pg72x ;

	pg72x->blockcount = 0 ;
	pg72x->samplecount = 0 ;

	switch (codec)
	{	case AU_H_G721_32 :
				g72x_writer_init (pg72x, G721_32_BITS_PER_SAMPLE) ;
				pg72x->bytesperblock = G721_32_BYTES_PER_BLOCK ;
				bitspersample = G721_32_BITS_PER_SAMPLE ;
				break ;

		case AU_H_G723_24:
				g72x_writer_init (pg72x, G723_24_BITS_PER_SAMPLE) ;
				pg72x->bytesperblock = G723_24_BYTES_PER_BLOCK ;
				bitspersample = G723_24_BITS_PER_SAMPLE ;
				break ;

		case AU_H_G723_40:
				g72x_writer_init (pg72x, G723_40_BITS_PER_SAMPLE) ;
				pg72x->bytesperblock = G723_40_BYTES_PER_BLOCK ;
				bitspersample = G723_40_BITS_PER_SAMPLE ;
				break ;

		default : return SFE_UNIMPLEMENTED ;
		} ;

	psf->write_short	= au_g72x_write_s ;
	psf->write_int		= au_g72x_write_i ;
	psf->write_float	= au_g72x_write_f ;
	psf->write_double	= au_g72x_write_d ;

 	psf->close = au_g72x_close ;

	psf->blockwidth = psf->bytewidth = 1 ;

	psf->filelength = psf_get_filelen (psf) ;
	if (psf->filelength < psf->dataoffset)
		psf->filelength = psf->dataoffset ;

	psf->datalength = psf->filelength - psf->dataoffset ;

	if (psf->datalength % pg72x->blocksize)
		pg72x->blocks = (psf->datalength / pg72x->blocksize) + 1 ;
	else
		pg72x->blocks = psf->datalength / pg72x->blocksize ;

	if (psf->datalength > 0)
		psf->sf.frames = (8 * psf->datalength) / bitspersample ;

	if ((psf->sf.frames * bitspersample) / 8 != psf->datalength)
		psf_log_printf (psf, "*** Warning : weird psf->datalength.\n") ;

	return 0 ;
} /* au_g72x_writer_init */

/*============================================================================================
** G721 Read Functions.
*/

static int
au_g72x_decode_block	(SF_PRIVATE *psf, G72x_DATA *pg72x)
{	int	k ;

	pg72x->blockcount ++ ;
	pg72x->samplecount = 0 ;

	if (pg72x->samplecount > pg72x->blocksize)
	{	memset (pg72x->samples, 0, G72x_BLOCK_SIZE * sizeof (short)) ;
		return 1 ;
		} ;

	if ((k = psf_fread (pg72x->block, 1, pg72x->bytesperblock, psf)) != pg72x->bytesperblock)
		psf_log_printf (psf, "*** Warning : short read (%d != %d).\n", k, pg72x->bytesperblock) ;

	pg72x->blocksize = k ;
	g72x_decode_block (pg72x) ;

	return 1 ;
} /* au_g72x_decode_block */

static int
au_g72x_read_block	(SF_PRIVATE *psf, G72x_DATA *pg72x, short *ptr, int len)
{	int	count, total = 0, indx = 0 ;

	while (indx < len)
	{	if (pg72x->blockcount >= pg72x->blocks && pg72x->samplecount >= pg72x->samplesperblock)
		{	memset (&(ptr [indx]), 0, (len - indx) * sizeof (short)) ;
			return total ;
			} ;

		if (pg72x->samplecount >= pg72x->samplesperblock)
			au_g72x_decode_block (psf, pg72x) ;

		count = pg72x->samplesperblock - pg72x->samplecount ;
		count = (len - indx > count) ? count : len - indx ;

		memcpy (&(ptr [indx]), &(pg72x->samples [pg72x->samplecount]), count * sizeof (short)) ;
		indx += count ;
		pg72x->samplecount += count ;
		total = indx ;
		} ;

	return total ;
} /* au_g72x_read_block */

static sf_count_t
au_g72x_read_s	(SF_PRIVATE *psf, short *ptr, sf_count_t len)
{	G72x_DATA 	*pg72x ;
	int			readcount, count ;
	sf_count_t	total = 0 ;

	if (! psf->fdata)
		return 0 ;
	pg72x = (G72x_DATA*) psf->fdata ;

	while (len > 0)
	{	readcount = (len > 0x10000000) ? 0x10000000 : (int) len ;

		count = au_g72x_read_block (psf, pg72x, ptr, readcount) ;

		total += count ;
		len -= count ;

		if (count != readcount)
			break ;
		} ;

	return total ;
} /* au_g72x_read_s */

static sf_count_t
au_g72x_read_i	(SF_PRIVATE *psf, int *ptr, sf_count_t len)
{	G72x_DATA *pg72x ;
	short		*sptr ;
	int			k, bufferlen, readcount = 0, count ;
	sf_count_t	total = 0 ;

	if (! psf->fdata)
		return 0 ;
	pg72x = (G72x_DATA*) psf->fdata ;

	sptr = psf->u.sbuf ;
	bufferlen = SF_BUFFER_LEN / sizeof (short) ;
	while (len > 0)
	{	readcount = (len >= bufferlen) ? bufferlen : len ;
		count = au_g72x_read_block (psf, pg72x, sptr, readcount) ;

		for (k = 0 ; k < readcount ; k++)
			ptr [total + k] = sptr [k] << 16 ;

		total += count ;
		len -= readcount ;
		if (count != readcount)
			break ;
		} ;

	return total ;
} /* au_g72x_read_i */

static sf_count_t
au_g72x_read_f	(SF_PRIVATE *psf, float *ptr, sf_count_t len)
{	G72x_DATA *pg72x ;
	short		*sptr ;
	int			k, bufferlen, readcount = 0, count ;
	sf_count_t	total = 0 ;
	float 		normfact ;

	if (! psf->fdata)
		return 0 ;
	pg72x = (G72x_DATA*) psf->fdata ;

	normfact = (psf->norm_float == SF_TRUE) ? 1.0 / ((float) 0x8000) : 1.0 ;

	sptr = psf->u.sbuf ;
	bufferlen = SF_BUFFER_LEN / sizeof (short) ;
	while (len > 0)
	{	readcount = (len >= bufferlen) ? bufferlen : len ;
		count = au_g72x_read_block (psf, pg72x, sptr, readcount) ;
		for (k = 0 ; k < readcount ; k++)
			ptr [total + k] = normfact * sptr [k] ;

		total += count ;
		len -= readcount ;
		if (count != readcount)
			break ;
		} ;

	return total ;
} /* au_g72x_read_f */

static sf_count_t
au_g72x_read_d	(SF_PRIVATE *psf, double *ptr, sf_count_t len)
{	G72x_DATA *pg72x ;
	short		*sptr ;
	int			k, bufferlen, readcount = 0, count ;
	sf_count_t	total = 0 ;
	double		normfact ;

	if (! psf->fdata)
		return 0 ;
	pg72x = (G72x_DATA*) psf->fdata ;

	normfact = (psf->norm_double == SF_TRUE) ? 1.0 / ((double) 0x8000) : 1.0 ;

	sptr = psf->u.sbuf ;
	bufferlen = SF_BUFFER_LEN / sizeof (short) ;
	while (len > 0)
	{	readcount = (len >= bufferlen) ? bufferlen : len ;
		count = au_g72x_read_block (psf, pg72x, sptr, readcount) ;
		for (k = 0 ; k < readcount ; k++)
			ptr [total + k] = normfact * (double) (sptr [k]) ;

		total += count ;
		len -= readcount ;
		if (count != readcount)
			break ;
		} ;

	return total ;
} /* au_g72x_read_d */

static sf_count_t
au_g72x_seek	(SF_PRIVATE *psf, int mode, sf_count_t offset)
{
	/* Prevent compiler warnings. */
	mode ++ ;
	offset ++ ;

	psf_log_printf (psf, "seek unsupported\n") ;

	/*	No simple solution. To do properly, would need to seek
	**	to start of file and decode everything up to seek position.
	**	Maybe implement SEEK_SET to 0 only?
	*/
	return 0 ;

/*
**		G72x_DATA	*pg72x ;
**		int			newblock, newsample, samplecount ;
**
**		if (! psf->fdata)
**			return 0 ;
**		pg72x = (G72x_DATA*) psf->fdata ;
**
**		if (! (psf->datalength && psf->dataoffset))
**		{	psf->error = SFE_BAD_SEEK ;
**			return	PSF_SEEK_ERROR ;
**			} ;
**
**		samplecount = (8 * psf->datalength) / G721_32_BITS_PER_SAMPLE ;
**
**		switch (whence)
**		{	case SEEK_SET :
**					if (offset < 0 || offset > samplecount)
**					{	psf->error = SFE_BAD_SEEK ;
**						return	PSF_SEEK_ERROR ;
**						} ;
**					newblock  = offset / pg72x->samplesperblock ;
**					newsample = offset % pg72x->samplesperblock ;
**					break ;
**
**			case SEEK_CUR :
**					if (psf->current + offset < 0 || psf->current + offset > samplecount)
**					{	psf->error = SFE_BAD_SEEK ;
**						return	PSF_SEEK_ERROR ;
**						} ;
**					newblock  = (8 * (psf->current + offset)) / pg72x->samplesperblock ;
**					newsample = (8 * (psf->current + offset)) % pg72x->samplesperblock ;
**					break ;
**
**			case SEEK_END :
**					if (offset > 0 || samplecount + offset < 0)
**					{	psf->error = SFE_BAD_SEEK ;
**						return	PSF_SEEK_ERROR ;
**						} ;
**					newblock  = (samplecount + offset) / pg72x->samplesperblock ;
**					newsample = (samplecount + offset) % pg72x->samplesperblock ;
**					break ;
**
**			default :
**					psf->error = SFE_BAD_SEEK ;
**					return	PSF_SEEK_ERROR ;
**			} ;
**
**		if (psf->mode == SFM_READ)
**		{	psf_fseek (psf, psf->dataoffset + newblock * pg72x->blocksize, SEEK_SET) ;
**			pg72x->blockcount  = newblock ;
**			au_g72x_decode_block (psf, pg72x) ;
**			pg72x->samplecount = newsample ;
**			}
**		else
**		{	/+* What to do about write??? *+/
**			psf->error = SFE_BAD_SEEK ;
**			return	PSF_SEEK_ERROR ;
**			} ;
**
**		psf->current = newblock * pg72x->samplesperblock + newsample ;
**		return psf->current ;
**
*/
} /* au_g72x_seek */

/*==========================================================================================
** G72x Write Functions.
*/

static int
au_g72x_encode_block	(SF_PRIVATE *psf, G72x_DATA *pg72x)
{	int k ;

	/* Encode the samples. */
	g72x_encode_block (pg72x) ;

	/* Write the block to disk. */
	if ((k = psf_fwrite (pg72x->block, 1, pg72x->blocksize, psf)) != pg72x->blocksize)
		psf_log_printf (psf, "*** Warning : short write (%d != %d).\n", k, pg72x->blocksize) ;

	pg72x->samplecount = 0 ;
	pg72x->blockcount ++ ;

	/* Set samples to zero for next block. */
	memset (pg72x->samples, 0, G72x_BLOCK_SIZE * sizeof (short)) ;

	return 1 ;
} /* au_g72x_encode_block */

static int
au_g72x_write_block	(SF_PRIVATE *psf, G72x_DATA *pg72x, const short *ptr, int len)
{	int	count, total = 0, indx = 0 ;

	while (indx < len)
	{	count = pg72x->samplesperblock - pg72x->samplecount ;

		if (count > len - indx)
			count = len - indx ;

		memcpy (&(pg72x->samples [pg72x->samplecount]), &(ptr [indx]), count * sizeof (short)) ;
		indx += count ;
		pg72x->samplecount += count ;
		total = indx ;

		if (pg72x->samplecount >= pg72x->samplesperblock)
			au_g72x_encode_block (psf, pg72x) ;
		} ;

	return total ;
} /* au_g72x_write_block */

static sf_count_t
au_g72x_write_s	(SF_PRIVATE *psf, const short *ptr, sf_count_t len)
{	G72x_DATA 	*pg72x ;
	int			writecount, count ;
	sf_count_t	total = 0 ;

	if (! psf->fdata)
		return 0 ;
	pg72x = (G72x_DATA*) psf->fdata ;

	while (len > 0)
	{	writecount = (len > 0x10000000) ? 0x10000000 : (int) len ;

		count = au_g72x_write_block (psf, pg72x, ptr, writecount) ;

		total += count ;
		len -= count ;
		if (count != writecount)
			break ;
		} ;

	return total ;
} /* au_g72x_write_s */

static sf_count_t
au_g72x_write_i	(SF_PRIVATE *psf, const int *ptr, sf_count_t len)
{	G72x_DATA *pg72x ;
	short		*sptr ;
	int			k, bufferlen, writecount = 0, count ;
	sf_count_t	total = 0 ;

	if (! psf->fdata)
		return 0 ;
	pg72x = (G72x_DATA*) psf->fdata ;

	sptr = psf->u.sbuf ;
	bufferlen = ((SF_BUFFER_LEN / psf->blockwidth) * psf->blockwidth) / sizeof (short) ;
	while (len > 0)
	{	writecount = (len >= bufferlen) ? bufferlen : len ;
		for (k = 0 ; k < writecount ; k++)
			sptr [k] = ptr [total + k] >> 16 ;
		count = au_g72x_write_block (psf, pg72x, sptr, writecount) ;

		total += count ;
		len -= writecount ;
		if (count != writecount)
			break ;
		} ;
	return total ;
} /* au_g72x_write_i */

static sf_count_t
au_g72x_write_f	(SF_PRIVATE *psf, const float *ptr, sf_count_t len)
{	G72x_DATA *pg72x ;
	short		*sptr ;
	int			k, bufferlen, writecount = 0, count ;
	sf_count_t	total = 0 ;
	float		normfact ;

	if (! psf->fdata)
		return 0 ;
	pg72x = (G72x_DATA*) psf->fdata ;

	normfact = (psf->norm_float == SF_TRUE) ? (1.0 * 0x8000) : 1.0 ;

	sptr = psf->u.sbuf ;
	bufferlen = ((SF_BUFFER_LEN / psf->blockwidth) * psf->blockwidth) / sizeof (short) ;
	while (len > 0)
	{	writecount = (len >= bufferlen) ? bufferlen : len ;
		for (k = 0 ; k < writecount ; k++)
			sptr [k] = lrintf (normfact * ptr [total + k]) ;
		count = au_g72x_write_block (psf, pg72x, sptr, writecount) ;

		total += count ;
		len -= writecount ;
		if (count != writecount)
			break ;
		} ;

	return total ;
} /* au_g72x_write_f */

static sf_count_t
au_g72x_write_d	(SF_PRIVATE *psf, const double *ptr, sf_count_t len)
{	G72x_DATA *pg72x ;
	short		*sptr ;
	int			k, bufferlen, writecount = 0, count ;
	sf_count_t	total = 0 ;
	double		normfact ;

	if (! psf->fdata)
		return 0 ;
	pg72x = (G72x_DATA*) psf->fdata ;

	normfact = (psf->norm_double == SF_TRUE) ? (1.0 * 0x8000) : 1.0 ;

	sptr = psf->u.sbuf ;
	bufferlen = ((SF_BUFFER_LEN / psf->blockwidth) * psf->blockwidth) / sizeof (short) ;
	while (len > 0)
	{	writecount = (len >= bufferlen) ? bufferlen : len ;
		for (k = 0 ; k < writecount ; k++)
			sptr [k] = lrint (normfact * ptr [total + k]) ;
		count = au_g72x_write_block (psf, pg72x, sptr, writecount) ;

		total += count ;
		len -= writecount ;
		if (count != writecount)
			break ;
		} ;

	return total ;
} /* au_g72x_write_d */

static int
au_g72x_close	(SF_PRIVATE *psf)
{	G72x_DATA *pg72x ;

	if (! psf->fdata)
		return 0 ;

	pg72x = (G72x_DATA*) psf->fdata ;

	if (psf->mode == SFM_WRITE)
	{	/*	If a block has been partially assembled, write it out
		**	as the final block.
		*/

		if (pg72x->samplecount && pg72x->samplecount < G72x_BLOCK_SIZE)
			au_g72x_encode_block (psf, pg72x) ;

		if (psf->write_header)
			psf->write_header (psf, SF_FALSE) ;
		} ;

	return 0 ;
} /* au_g72x_close */

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 3cc5439e-7247-486b-b2e6-11a4affa5744
*/
