/*
** Copyright (C) 2006-2016 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (C) 2006 Paul Davis <paul@linuxaudiosystems.com>
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
#include <stddef.h>
#include <string.h>

#include "common.h"

/* i'm not sure if axml.c is necessary. it seems braodcast.c handles the coding
** history size, which may not be necessary for BW64
*/

SF_AXML_INFO_16K*
axml_var_alloc (void)
{	return calloc (1, sizeof (SF_AXML_INFO_16K)) ;
} /* amxl_var_alloc */

int
axml_var_set (SF_PRIVATE *psf, const SF_AXML_INFO * info, size_t datasize)
{	size_t len ;

	if (info == NULL)
		return SF_FALSE ;

	if (bc_min_size (info) > datasize)
	{	psf->error = SFE_BAD_AXML_INFO_SIZE ;
		return SF_FALSE ;
		} ;

	if (datasize >= sizeof (SF_AXML_INFO_16K))
	{	psf->error = SFE_BAD_AXML_INFO_TOO_BIG ;
		return SF_FALSE ;
		} ;

	if (psf->axml_16k == NULL)
	{	if ((psf->axml_16k = axml_var_alloc ()) == NULL)
		{	psf->error = SFE_MALLOC_FAILED ;
			return SF_FALSE ;
			} ;
		} ;


int
axml_var_get (SF_PRIVATE *psf, SF_AXML_INFO * data, size_t datasize)
{	size_t size ;

	if (psf->axml_16k == NULL)
		return SF_FALSE ;

	size = SF_MIN (datasize, bc_min_size ((const SF_AXML_INFO *) psf->axml_16k)) ;

	memcpy (data, psf->axml_16k, size) ;

	return SF_TRUE ;
}
/* axml_var_get */
