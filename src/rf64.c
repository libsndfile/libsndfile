/*
** Copyright (C) 2008 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include	"sndfile.h"
#include	"sfendian.h"
#include	"common.h"

/*------------------------------------------------------------------------------
** Macros to handle big/little endian issues.
*/
#define	RF64_MARKER		MAKE_MARKER ('R', 'F', '6', '4')
#define	WAVE_MARKER		MAKE_MARKER ('W', 'A', 'V', 'E')
#define	ds64_MARKER		MAKE_MARKER ('d', 's', '6', '4')
#define	fmt_MARKER		MAKE_MARKER ('f', 'm', 't', ' ')
#define	data_MARKER		MAKE_MARKER ('d', 'a', 't', 'a')


/*------------------------------------------------------------------------------
** Typedefs.
*/

/*------------------------------------------------------------------------------
** Private static functions.
*/

static int	rf64_read_header (SF_PRIVATE *psf) ;

/*------------------------------------------------------------------------------
** Public function.
*/

int
rf64_open (SF_PRIVATE *psf)
{	int	subformat, error = 0 ;

puts (__func__) ;

	if (psf->mode == SFM_WRITE || psf->mode == SFM_RDWR)
		return SFE_UNIMPLEMENTED ;

	if ((error = rf64_read_header (psf)))
			return error ;

	if ((psf->sf.format & SF_FORMAT_TYPEMASK) != SF_FORMAT_RF64)
		return	SFE_BAD_OPEN_FORMAT ;

	subformat = psf->sf.format & SF_FORMAT_SUBMASK ;

	return error ;
} /* rf64_open */

/*------------------------------------------------------------------------------
*/

static int
rf64_read_header (SF_PRIVATE *psf)
{	int marker ;

	/* Set position to start of file to begin reading header. */
	psf_binheader_readf (psf, "pm", 0, &marker) ;
	if (marker != RF64_MARKER)
		return SFE_WVE_NOT_WVE ;


	return 0 ;
} /* rf64_read_header */


