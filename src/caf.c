/*
** Copyright (C) 2005 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<ctype.h>

#include	"sndfile.h"
#include	"config.h"
#include	"sfendian.h"
#include	"common.h"

#if (ENABLE_EXPERIMENTAL_CODE == 1)

int
caf_open	(SF_PRIVATE *psf)
{	if (psf)
		return SFE_UNIMPLEMENTED ;
	return (psf && 0) ;
} /* caf_open */

#else

/*------------------------------------------------------------------------------
** Macros to handle big/little endian issues.
*/

#define aac_MARKER		MAKE_MARKER ('a', 'a', 'c', ' ')
#define alac_MARKER		MAKE_MARKER ('a', 'l', 'a', 'c')
#define alaw_MARKER		MAKE_MARKER ('a', 'l', 'a', 'w')
#define caff_MARKER		MAKE_MARKER ('c', 'a', 'f', 'f')
#define data_MARKER		MAKE_MARKER ('c', 'a', 'f', 'f')
#define desc_MARKER		MAKE_MARKER ('d', 'a', 't', 'a')
#define free_MARKER		MAKE_MARKER ('f', 'r', 'e', 'e')
#define kuki_MARKER		MAKE_MARKER ('k', 'u', 'k', 'i')
#define lpcm_MARKER		MAKE_MARKER ('l', 'p', 'c', 'm')
#define pakt_MARKER		MAKE_MARKER ('p', 'a', 'k', 't')
#define ima4_MARKER		MAKE_MARKER ('i', 'm', 'a', '4')
#define ulaw_MARKER		MAKE_MARKER ('u', 'l', 'a', 'w')
#define MAC3_MARKER		MAKE_MARKER ('M', 'A', 'C', '3')
#define MAC6_MARKER		MAKE_MARKER ('M', 'A', 'C', '6')

#define SFE_CAF_NOT_CAF	666
#define SFE_CAF_NO_DESC	667

/*------------------------------------------------------------------------------
** Typedefs.
*/

/*------------------------------------------------------------------------------
** Private static functions.
*/

static int	caf_read_header (SF_PRIVATE *psf) ;

/*------------------------------------------------------------------------------
** Public function.
*/

int
caf_open (SF_PRIVATE *psf)
{	int	subformat, error = 0 ;

	if (psf->mode == SFM_WRITE || psf->mode == SFM_RDWR)
		return SFE_UNIMPLEMENTED ;

	if ((error = caf_read_header (psf)))
			return error ;

	if ((psf->sf.format & SF_FORMAT_TYPEMASK) != SF_FORMAT_CAF)
		return	SFE_BAD_OPEN_FORMAT ;

	subformat = psf->sf.format & SF_FORMAT_SUBMASK ;

	return error ;
} /* caf_open */

/*------------------------------------------------------------------------------
*/

static int
caf_read_header (SF_PRIVATE *psf)
{	short version, flags ;
	sf_count_t size ;
	int marker ;

	/* Set position to start of file to begin reading header. */
	psf_binheader_readf (psf, "pmE2E2", 0, &marker, &version, &flags) ;
	psf_log_printf (psf, "%M\n  Version : %d\n  Flags   : %x\n", marker, version, flags) ;
	if (marker != caff_MARKER)
		return SFE_CAF_NOT_CAF ;

	psf_binheader_readf (psf, "mE8", &marker, &size) ;
	psf_log_printf (psf, " %M : %D\n\n", marker, size) ;
	if (marker != desc_MARKER)
		return SFE_CAF_NO_DESC ;


	psf->dataoffset = 0x1000 ;
	psf->datalength = psf->filelength - psf->dataoffset ;

	psf->sf.format		= SF_FORMAT_CAF | SF_FORMAT_ALAW ;
	psf->sf.samplerate	= 8000 ;
	psf->sf.frames		= psf->datalength ;
	psf->sf.channels	= 1 ;

	return alaw_init (psf) ;
} /* caf_read_header */

/*------------------------------------------------------------------------------
*/

#endif
/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 65883e65-bd3c-4618-9241-d3c02fd630bd
*/
