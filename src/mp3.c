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
int mp3_close_lame (SF_PRIVATE *psf) ;

/*------------------------------------------------------------------------------
** Public function.
*/

int
mp3_open (SF_PRIVATE *psf)
{
	int	subformat, error = 0 ;
	MP3_PRIVATE* mdata ;
	printf ("*** in mp3_open\n") ;
	if ((mdata = calloc (1, sizeof (MP3_PRIVATE))) == NULL)
		return SFE_MALLOC_FAILED ;
	psf->container_data = mdata ;

	if (psf->file.mode == SFM_RDWR)
		return SFE_BAD_MODE_RW ;

	if (psf->file.mode == SFM_WRITE)
	{
		printf ("*** callinf mp3_open_lame\n") ;
		return mp3_open_lame (psf) ;
	}
	if (psf->file.mode == SFM_RDWR)
		return SFE_UNIMPLEMENTED ;

	return error ;
	if (psf->file.mode == SFM_READ)
		return SFE_UNIMPLEMENTED ;

	return error ;
} /* mp3_open */

/*------------------------------------------------------------------------------
*/

int
mp3_open_lame (SF_PRIVATE *psf)
{
	MP3_PRIVATE *p = (MP3_PRIVATE*) psf->container_data ;
	lame_global_flags *gfp = p->gfp = lame_init () ;
	int error = 0 , format ;
	printf ("** in mp3_open_lame\n") ;
	format = SF_CONTAINER (psf->sf.format) ;
	if (format != SF_FORMAT_MP3)
		return SFE_BAD_OPEN_FORMAT ;
	lame_set_num_channels (gfp, psf->sf.channels) ;
	lame_set_in_samplerate (gfp, psf->sf.samplerate) ;
	lame_set_brate (gfp, 256) ;	/* FIXME to parameter */
	lame_set_mode (gfp, 1) ;	/* FIXME to parameter */
	lame_set_quality (gfp, 2) ;	/* 2=high	5 = medium  7=low */
	if ((error = lame_init_params (gfp)) <0)
		return SFE_BAD_OPEN_FORMAT ;
	if ((p->mp3buffer = calloc (1, MP3BUFFER_SIZE)) == NULL)
		return SFE_MALLOC_FAILED ;
	if ((p->mp3data_l = calloc (1, MP3DATA_SIZE*sizeof (double))) == NULL)
		return SFE_MALLOC_FAILED ;
	if ((p->mp3data_r = calloc (1, MP3DATA_SIZE*sizeof (double))) == NULL)
		return SFE_MALLOC_FAILED ;
	//psf->command = mp3_command ;
	psf->datalength = 0 ;
	return error ;
}

int
mp3_close_lame (SF_PRIVATE *psf)
{
	MP3_PRIVATE *p = psf->container_data ;
	lame_global_flags *gfp = p->gfp ;
	int bytes = lame_encode_flush (gfp, p->mp3buffer, p->mp3buffer_size) ;
	if (bytes > 0) psf_fwrite (p->mp3buffer, 1, bytes, psf) ;
	//lame_mp3_tags_fid (gfp, p->fout) ;
	lame_close (gfp) ;

	free (p->mp3buffer) ;
	free (p->mp3data_l) ;
	free (p->mp3data_r) ;
	p->gfp = NULL ;
	return 0 ;

}

/*------------------------------------------------------------------------------
*/

#endif
