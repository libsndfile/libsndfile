/*
** Copyright (C) 2008-2012 Erik de Castro Lopo <erikd@mega-nerd.com>
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


int
psf_store_read_chunk (READ_CHUNKS * pchk, int64_t marker, sf_count_t offset, uint32_t len)
{
	if (pchk->count == 0)
	{	pchk->used = 0 ;
		pchk->count = 20 ;
		pchk->chunks = calloc (pchk->count, sizeof (READ_CHUNK)) ;
		}
	else if (pchk->used > pchk->count)
		return SFE_INTERNAL ;
	else if (pchk->used == pchk->count)
	{	READ_CHUNK * old_ptr = pchk->chunks ;
		int new_count = 3 * (pchk->count + 1) / 2 ;

		pchk->chunks = realloc (old_ptr, new_count * sizeof (READ_CHUNK)) ;
		if (pchk->chunks == NULL)
		{	pchk->chunks = old_ptr ;
			return SFE_MALLOC_FAILED ;
			} ;
		pchk->count = new_count ;
		} ;

	pchk->chunks [pchk->used].marker = marker ;
	pchk->chunks [pchk->used].offset = offset ;
	pchk->chunks [pchk->used].len = len ;

	pchk->used ++ ;

	return SFE_NO_ERROR ;
} /* psf_store_read_chunk */

int
psf_save_write_chunk (WRITE_CHUNKS * pchk, int64_t marker, const SF_CHUNK_INFO * chunk_info)
{	uint32_t len ;

	if (pchk->count == 0)
	{	pchk->used = 0 ;
		pchk->count = 20 ;
		pchk->chunks = calloc (pchk->count, sizeof (WRITE_CHUNK)) ;
		}
	else if (pchk->used >= pchk->count)
	{	WRITE_CHUNK * old_ptr = pchk->chunks ;
		int new_count = 3 * (pchk->count + 1) / 2 ;

		pchk->chunks = realloc (old_ptr, new_count * sizeof (WRITE_CHUNK)) ;
		if (pchk->chunks == NULL)
		{	pchk->chunks = old_ptr ;
			return SFE_MALLOC_FAILED ;
			} ;
		} ;

	len = chunk_info->datalen ;
	while (len & 3) len ++ ;

	pchk->chunks [pchk->used].marker = marker ;
	pchk->chunks [pchk->used].len = len ;
	pchk->chunks [pchk->used].data = psf_memdup (chunk_info->data, chunk_info->datalen) ;

	pchk->used ++ ;

	return SFE_NO_ERROR ;
} /* psf_store_read_chunk */

int
psf_find_read_chunk (READ_CHUNKS * pchk, int64_t marker)
{	uint32_t k ;

	for (k = 0 ; k < pchk->used ; k++)
		if (pchk->chunks [k].marker == marker)
			return k ;

	return -1 ;
} /* psf_find_read_chunk */
