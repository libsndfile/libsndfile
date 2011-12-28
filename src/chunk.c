/*
** Copyright (C) 2008-2011 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include	<stdlib.h>
#include	<string.h>

#include	"sndfile.h"
#include	"sfendian.h"
#include	"common.h"


void
psf_chunk_store (PRIV_CHUNK_LOG * pchk, int marker, sf_count_t offset, sf_count_t len)
{
	if (pchk->used >= pchk->count)
		return ;

	pchk->chunks [pchk->used].marker = marker ;
	pchk->chunks [pchk->used].offset = offset ;
	pchk->chunks [pchk->used].len = len ;

	pchk->used ++ ;

	return ;
} /* psf_chunk_store */

int
psf_chunk_find (PRIV_CHUNK_LOG * pchk, int marker)
{	int k ;

	for (k = 0 ; k < pchk->used ; k++)
		if (pchk->chunks [k].marker == marker)
			return k ;

	return -1 ;
} /* psf_chunk_find */

