/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2007             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: utility functions for vorbis codec test suite.
 last mod: $Id: util.c 13293 2007-07-24 00:09:47Z erikd $

 ********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>

#include "util.h"

void
set_data_in (float * data, unsigned len, float value)
{	unsigned k ;

	for (k = 0 ; k < len ; k++)
		data [k] = value ;
} /* set_data_in */

int
find_max_pos_peaks (PAIRS * pairs, unsigned count, const float * data, unsigned datalen)
{	unsigned di ;

	memset (pairs, 0, count * sizeof (pairs [0])) ;
	data = NULL ;

	for (di = 1 ; di < datalen - 1 ; di++)
	{
	
		} ;

	return 0 ;
}
