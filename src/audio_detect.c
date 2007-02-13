/*
** Copyright (C) 1999-2006 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <string.h>
#include <errno.h>

#include "common.h"
#include "sfendian.h"

typedef struct
{
	int smean, umean ;
	signed char smin, smax ;
	unsigned char umin, umax ;
} M_STAT ;

typedef struct
{	M_STAT three [3] ;
	M_STAT four [4] ;
} DATA_STATS ;


static void generate_stats (DATA_STATS * stats, const unsigned char * data, int datalen) ;

int
audio_detect (SF_PRIVATE * psf, AUDIO_DETECT *ad, const unsigned char * data, int datalen)
{	DATA_STATS stats ;

	if (psf == NULL)
		return 0 ;

	if (ad == NULL || datalen < 256)
	{	printf ("datalen %d\n", datalen) ;
		return 0 ;
		} ;

	generate_stats (&stats, data, datalen) ;

	if (ad->endianness == SF_ENDIAN_LITTLE
		&& stats.four [0].umean == 0 && stats.four [1].umean > 0 && stats.four [2].umean > 0 && stats.four [3].umean > 0)
	{	/* Almost certainly 24 bit data stored in 32 bit ints. */
		return SF_FORMAT_PCM_32 ;
		} ;

	if (ad->endianness == SF_ENDIAN_LITTLE && stats.four [3].umin > 0x43 && stats.four [3].umax < 0x46)
	{	/* Almost certainly 32 bit floats. */
		return SF_FORMAT_FLOAT ;
		} ;

	return 0 ;
} /* data_detect */

static void
generate_stats (DATA_STATS * stats, const unsigned char * data, int datalen)
{
	int k, indx ;

	memset (stats, 0, sizeof (stats [0])) ;

	/* Make sure datalen has both 3 and 4 as a factor. */
	datalen -= datalen % (3 * 4) ;

	for (k = 0 ; k < 3 ; k++)
	{	stats->three [k].smin = data [k] ;
		stats->three [k].smax = data [k] ;
		stats->three [k].umin = data [k] ;
		stats->three [k].umax = data [k] ;
		} ;

	for (k = 0 ; k < 4 ; k++)
	{	stats->four [k].smin = data [k] ;
		stats->four [k].smax = data [k] ;
		stats->four [k].umin = data [k] ;
		stats->four [k].umax = data [k] ;
		} ;

	for (k = 0 ; k < datalen ; k++)
	{	signed char schar = data [k] ;
		unsigned char uchar = data [k] ;
	
		indx = k % 3 ;
		stats->three [indx].smean += schar ;
		stats->three [indx].umean += uchar ;
		stats->three [indx].smin = SF_MIN (stats->three [indx].smin, schar) ;
		stats->three [indx].smax = SF_MAX (stats->three [indx].smax, schar) ;
		stats->three [indx].umin = SF_MIN (stats->three [indx].umin, uchar) ;
		stats->three [indx].umax = SF_MAX (stats->three [indx].umax, uchar) ;

		indx = k % 4 ;
		stats->four [indx].smean += schar ;
		stats->four [indx].umean += uchar ;
		stats->four [indx].smin = SF_MIN (stats->four [indx].smin, schar) ;
		stats->four [indx].smax = SF_MAX (stats->four [indx].smax, schar) ;
		stats->four [indx].umin = SF_MIN (stats->four [indx].umin, uchar) ;
		stats->four [indx].umax = SF_MAX (stats->four [indx].umax, uchar) ;
		} ;

	for (k = 0 ; k < 3 ; k++)
	{	stats->three [k].smean /= datalen / 3 ;
		stats->three [k].umean /= datalen / 3 ;
		} ;

	for (k = 0 ; k < 4 ; k++)
	{	stats->four [k].smean /= datalen / 4 ;
		stats->four [k].umean /= datalen / 4 ;
		} ;

	return ;
} /* generate_stats */

