/*
** Copyright (C) 2002-2011 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (C) 2019 John ffitch
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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
#include	<math.h>

#include	"sndfile.h"
#include	"sfendian.h"
#include	"common.h"
#include	"mp3.h"

#if 0
//(ENABLE_EXPERIMENTAL_CODE == 0)

int
mp3_open	(SF_PRIVATE *psf)
{	if (psf)
		return SFE_UNIMPLEMENTED ;
	return (psf && 0) ;
} /* mp3_open */

#else

/*------------------------------------------------------------------------------
** Typedefs.
*/

/*------------------------------------------------------------------------------
** Private static functions.
*/

static int mp3_open_lame (SF_PRIVATE *psf) ;
static int mp3_close_lame (SF_PRIVATE *psf) ;

static sf_count_t mp3_lame_write_short (SF_PRIVATE *psf, const short *ptr, sf_count_t items) ;
static sf_count_t mp3_lame_write_int (SF_PRIVATE *psf, const int *ptr, sf_count_t items) ;
static sf_count_t mp3_lame_write_float (SF_PRIVATE *psf, const float *ptr, sf_count_t items) ;
static sf_count_t mp3_lame_write_double (SF_PRIVATE *psf, const double *ptr, sf_count_t items) ;
static sf_count_t mp3_lame_write_short_M (SF_PRIVATE *psf, const short *ptr, sf_count_t items) ;
static sf_count_t mp3_lame_write_int_M (SF_PRIVATE *psf, const int *ptr, sf_count_t items) ;
static sf_count_t mp3_lame_write_float_M (SF_PRIVATE *psf, const float *ptr, sf_count_t items) ;
static sf_count_t mp3_lame_write_double_M (SF_PRIVATE *psf, const double *ptr, sf_count_t items) ;
static int mp3_command (SF_PRIVATE * psf, int command, void * data, int datasize) ;

/*------------------------------------------------------------------------------
** Public function.
*/

int
mp3_open (SF_PRIVATE *psf)
{	int	subformat, error = 0 ;
	MP3_PRIVATE* mdata ;
	if ((mdata = calloc (1, sizeof (MP3_PRIVATE))) == NULL)
		return SFE_MALLOC_FAILED ;
	psf->container_data = mdata ;

	if (psf->file.mode == SFM_RDWR)
		return SFE_BAD_MODE_RW ;


	if (psf->file.mode == SFM_WRITE)
	{

		return mp3_open_lame (psf) ;
	}

	if (psf->file.mode == SFM_RDWR)
		return SFE_UNIMPLEMENTED ;


	if (psf->file.mode == SFM_READ)
		return SFE_UNIMPLEMENTED ;

	return error ;
} /* mp3_open */

/*------------------------------------------------------------------------------
*/

static int
mp3_open_lame (SF_PRIVATE *psf)
{	MP3_PRIVATE *p = (MP3_PRIVATE*) psf->container_data ;
	lame_global_flags *gfp = p->gfp = lame_init () ;
	int error = 0, format, mode ;

	format = SF_CONTAINER (psf->sf.format) ;
	if (format != SF_FORMAT_MP3)
		return SFE_BAD_OPEN_FORMAT ;
	lame_set_num_channels (gfp, psf->sf.channels) ;
	lame_set_in_samplerate (gfp, psf->sf.samplerate) ;
	lame_set_brate (gfp, 256) ;	/* FIXME to parameter */
	mode = (psf->sf.format-SF_FORMAT_MP3_STEREO) & SF_FORMAT_SUBMASK ;
	lame_set_mode (gfp, mode) ;
	p->quality = 2 ;
	//lame_set_quality (gfp, 2) ;	/* 2=high	5 = medium  7=low */
	if ((p->mp3buffer = calloc (1, MP3BUFFER_SIZE)) == NULL)
		return SFE_MALLOC_FAILED ;
	psf->command = mp3_command ;
	psf->datalength = 0 ;	/* What is this?? */
	psf->dataoffset = 0 ;
	if (mode == 3)		/* MONO may overwrite data */
	{	psf->write_short = mp3_lame_write_short_M ;
		psf->write_int = mp3_lame_write_int_M ;
		psf->write_float = mp3_lame_write_float_M ;
		psf->write_double = mp3_lame_write_double_M ;
		if ((p->mp3data = calloc (2, MP3DATA_SIZE*sizeof (double))) == NULL)
			return SFE_MALLOC_FAILED ;
		}
	else
	{	psf->write_short = mp3_lame_write_short ;
		psf->write_int = mp3_lame_write_int ;
		psf->write_float = mp3_lame_write_float ;
		psf->write_double = mp3_lame_write_double ;
		}
	psf->container_close = mp3_close_lame ;
	psf->command = mp3_command ;
	return error ;
}

static int
mp3_close_lame (SF_PRIVATE *psf)
{	MP3_PRIVATE *p = psf->container_data ;
	lame_global_flags *gfp = p->gfp ;
	int bytes = lame_encode_flush (gfp, p->mp3buffer, MP3BUFFER_SIZE) ;
	if (bytes > 0) psf_fwrite (p->mp3buffer, 1, bytes, psf) ;
	//lame_mp3_tags_fid (gfp, p->fout) ;
	lame_close (gfp) ;

	free (p->mp3buffer) ;
	p->gfp = NULL ;
	return 0 ;
}

/*------------------------------------------------------------------------------
*/

static int
mp3_lame_parameters (lame_global_flags *gfp, MP3_PRIVATE *p)
{	int error ;
	lame_set_quality (gfp, p->quality) ;
	if ((error = lame_init_params (gfp)) < 0)
		return SFE_BAD_OPEN_FORMAT ;
	p->initialised = 1 ;
	return 1 ;
}

/* **** Note that if mode is MONO (3) then left and right channels are averaged and the data is changed.  Hence the _M functions **** */
static sf_count_t
mp3_lame_write_short (SF_PRIVATE *psf, const short int *ptr, sf_count_t items)
{	MP3_PRIVATE *p = psf->container_data ;
	lame_global_flags *gfp = p->gfp ;

	int bytes, count ;
	items /= psf->sf.channels ;
	if (p->initialised == 0)
		mp3_lame_parameters (gfp, p) ;
	do
	{	count = (items > MP3DATA_SIZE ? MP3DATA_SIZE : items) ;
		if (psf->sf.channels == 2)
		bytes = lame_encode_buffer_interleaved (gfp, (short *) ptr,
								count, p->mp3buffer,
								MP3BUFFER_SIZE) ;
		else
			bytes = lame_encode_buffer (gfp, (short *) ptr, (short *) ptr,
							count, p->mp3buffer,
							MP3BUFFER_SIZE) ;
		if (bytes > 0) psf_fwrite (p->mp3buffer, 1, bytes, psf) ;
		if (bytes < 0) return 0 ;
		ptr += count * psf->sf.channels ;
		items -= count ;
		} while (count > 0) ;
	return 1 ;
}

static sf_count_t
mp3_lame_write_int (SF_PRIVATE *psf, const int *ptr, sf_count_t items)
{	MP3_PRIVATE *p = psf->container_data ;
	lame_global_flags *gfp = p->gfp ;

	int bytes, count ;
	items /= psf->sf.channels ;
	if (p->initialised == 0)
		mp3_lame_parameters (gfp, p) ;
	do
	{	count = (items > MP3DATA_SIZE ? MP3DATA_SIZE : items) ;
		if (psf->sf.channels == 2)
			bytes = lame_encode_buffer_interleaved_int (gfp, ptr,
								count, p->mp3buffer,
								MP3BUFFER_SIZE) ;
		else
			bytes = lame_encode_buffer_int (gfp, ptr, ptr,
								count, p->mp3buffer,
								MP3BUFFER_SIZE) ;
		if (bytes > 0) psf_fwrite (p->mp3buffer, 1, bytes, psf) ;
		if (bytes < 0) return 0 ;
		ptr += count * psf->sf.channels ;
		items -= count ;
		} while (count > 0) ;
	return 1 ;
}

static sf_count_t
mp3_lame_write_float (SF_PRIVATE *psf, const float *ptr, sf_count_t items)
{	MP3_PRIVATE *p = psf->container_data ;
	lame_global_flags *gfp = p->gfp ;

	int bytes, count ;
	items /= psf->sf.channels ;
	if (p->initialised == 0)
		if (mp3_lame_parameters (gfp, p) == 0) return 0 ;
	do
	{	count = (items > MP3DATA_SIZE ? MP3DATA_SIZE : items) ;
		if (psf->sf.channels == 2)
			bytes = lame_encode_buffer_interleaved_ieee_float (gfp, ptr,
							count, p->mp3buffer,
							MP3BUFFER_SIZE) ;
		else
			bytes = lame_encode_buffer_ieee_float (gfp, ptr, ptr,
							count, p->mp3buffer,
							MP3BUFFER_SIZE) ;
		if (bytes > 0) psf_fwrite (p->mp3buffer, 1, bytes, psf) ;
		if (bytes < 0) return 0 ;
		items -= count ;
		ptr += count * psf->sf.channels ;
		} while (count > 0) ;
	return 1 ;
}

static sf_count_t
mp3_lame_write_double (SF_PRIVATE *psf, const double *ptr, sf_count_t items)
{	MP3_PRIVATE *p = psf->container_data ;
	lame_global_flags *gfp = p->gfp ;

	int bytes, count ;
	items /= psf->sf.channels ;
	if (p->initialised == 0)
		mp3_lame_parameters (gfp, p) ;
	do
	{	count = (items > MP3DATA_SIZE ? MP3DATA_SIZE : items) ;
		if (psf->sf.channels == 2)
			bytes = lame_encode_buffer_interleaved_ieee_double (gfp, ptr,
								count, p->mp3buffer,
								MP3BUFFER_SIZE) ;
		else
			bytes = lame_encode_buffer_ieee_double (gfp, ptr, ptr,
								count, p->mp3buffer,
								MP3BUFFER_SIZE) ;
		if (bytes > 0) psf_fwrite (p->mp3buffer, 1, bytes, psf) ;
		if (bytes < 0) return 0 ;
		items -= count ;
		ptr += count * psf->sf.channels ;
		} while (count > 0) ;
	return 1 ;
}
/* **************** MONO cases to avoid overwriting ************ */

static sf_count_t
mp3_lame_write_short_M (SF_PRIVATE *psf, const short *ptr, sf_count_t items)
{	MP3_PRIVATE *p = psf->container_data ;
	lame_global_flags *gfp = p->gfp ;

	int bytes, count, i ;
	short *left = (short *) p->mp3data ;
	short *right = left + MP3DATA_SIZE ;
	items /= psf->sf.channels ;
	if (p->initialised == 0)
		mp3_lame_parameters (gfp, p) ;
	do
	{	count = (items > MP3DATA_SIZE ? MP3DATA_SIZE : items) ;
		if (psf->sf.channels == 2)
		{	for (i = 0 ; i < MP3DATA_SIZE ; i++)
			{	left [i] = *ptr++ ;
				right [i] = *ptr++ ;
				}
			}
		else
		{	for (i = 0 ; i < MP3DATA_SIZE ; i++)
				left [i] = right [i] = *ptr++ ;
			}
		bytes = lame_encode_buffer (gfp, left, right,
						count, p->mp3buffer,
						MP3BUFFER_SIZE) ;
		if (bytes > 0) psf_fwrite (p->mp3buffer, 1, bytes, psf) ;
		if (bytes < 0) return 0 ;
		ptr += count * psf->sf.channels ;
		items -= count ;
		} while (count > 0) ;
	return 1 ;
}

static	sf_count_t
mp3_lame_write_int_M (SF_PRIVATE *psf, const int *ptr, sf_count_t items)
{	MP3_PRIVATE *p = psf->container_data ;
	lame_global_flags *gfp = p->gfp ;

	int bytes, count, i ;
	int *left = (int *) p->mp3data ;
	int *right = left+MP3DATA_SIZE ;
	items /= psf->sf.channels ;
	if (p->initialised == 0)
		mp3_lame_parameters (gfp, p) ;
	do
	{	count = (items > MP3DATA_SIZE ? MP3DATA_SIZE : items) ;
		if (psf->sf.channels == 2)
		{	for (i = 0 ; i < MP3DATA_SIZE ; i++)
			{	left [i] = *ptr++ ;
				right [i] = *ptr++ ;
				}
			}
		else
		{	for (i = 0 ; i < MP3DATA_SIZE ; i++)
				left [i] = right [i] = *ptr++ ;
			}
		bytes = lame_encode_buffer_int (gfp, left, right,
						count, p->mp3buffer,
						MP3BUFFER_SIZE) ;
		if (bytes > 0) psf_fwrite (p->mp3buffer, 1, bytes, psf) ;
		if (bytes < 0) return 0 ;
		ptr += count * psf->sf.channels ;
		items -= count ;
		} while (count > 0) ;
	return 1 ;
}
static sf_count_t
mp3_lame_write_float_M (SF_PRIVATE *psf, const float *ptr, sf_count_t items)
{	MP3_PRIVATE *p = psf->container_data ;
	lame_global_flags *gfp = p->gfp ;

	int bytes, count, i ;
	float *left = (float *) p->mp3data ;
	float *right = left+MP3DATA_SIZE ;
	items /= psf->sf.channels ;
	if (p->initialised == 0)
		mp3_lame_parameters (gfp, p) ;
	do
	{	count = (items > MP3DATA_SIZE ? MP3DATA_SIZE : items) ;
		if (psf->sf.channels == 2)
		{	for (i = 0 ; i < MP3DATA_SIZE ; i++)
			{	left [i] = *ptr++ ;
				right [i] = *ptr++ ;
				}
			}
		else
		{	for (i = 0 ; i < MP3DATA_SIZE ; i++)
				left [i] = right [i] = *ptr++ ;
			}
		bytes = lame_encode_buffer_ieee_float (gfp, left, right,
						count, p->mp3buffer,
						MP3BUFFER_SIZE) ;
		if (bytes > 0) psf_fwrite (p->mp3buffer, 1, bytes, psf) ;
		if (bytes < 0) return 0 ;
		ptr += count ;
		items -= count * psf->sf.channels ;
		} while (count > 0) ;
	return 1 ;
}

static sf_count_t
mp3_lame_write_double_M (SF_PRIVATE *psf, const double *ptr, sf_count_t items)
{	MP3_PRIVATE *p = psf->container_data ;
	lame_global_flags *gfp = p->gfp ;

	int bytes, count, i ;
	double *left = (double *) p->mp3data ;
	double *right = left+MP3DATA_SIZE ;
	items /= psf->sf.channels ;
	if (p->initialised == 0)
		mp3_lame_parameters (gfp, p) ;
	do
	{	count = (items > MP3DATA_SIZE ? MP3DATA_SIZE : items) ;
		if (psf->sf.channels == 2)
		{	for (i = 0 ; i < MP3DATA_SIZE ; i++)
			{	left [i] = *ptr++ ;
				right [i] = *ptr++ ;
				}
			}
		else
		{	for (i = 0 ; i < MP3DATA_SIZE ; i++)
				left [i] = right [i] = *ptr++ ;
			}
		bytes = lame_encode_buffer_ieee_double (gfp, left, right,
						count, p->mp3buffer,
						MP3BUFFER_SIZE) ;
		if (bytes > 0) psf_fwrite (p->mp3buffer, 1, bytes, psf) ;
		if (bytes < 0) return 0 ;
		items -= count ;
		ptr += count * psf->sf.channels ;
		} while (count > 0) ;
	return 1 ;
}

/* ********************************************************* */

static int
mp3_command (SF_PRIVATE * psf, int command, void * data, int datasize)
{	MP3_PRIVATE* p = (MP3_PRIVATE*) psf->container_data ;
	double quality ;

	switch (command)
	{	case SFC_SET_QUALITY_LEVEL :
			if (data == NULL || datasize != sizeof (double))
				return SF_FALSE ;

			if (psf->have_written)
				return SF_FALSE ;

			/* MP3 quality is in the range  [0, 9] while libsndfile takes
			** values in the range [0.0, 1.0]. Massage the libsndfile value here.
			*/
			quality = (*((double *) data)) * 9.0 ;
			/* Clip range. */
			p->quality = lrint (SF_MAX (0.0, SF_MIN (9.0, quality))) ;
			lame_set_quality (p->gfp, p->quality) ;

			psf_log_printf (psf, "%s : Setting SFC_SET_QUALITY_LEVEL to %u.\n", __func__, p->quality) ;

			return SF_TRUE ;

		default :
			return SF_FALSE ;
		} ;

	return SF_FALSE ;
} /* mp3_command */



#endif
