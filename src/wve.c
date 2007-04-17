/*
** Copyright (C) 2002-2007 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (C) 2007 Reuben Thomas
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
** Macros to handle big/little endian issues, and other magic numbers.
*/

#define ALAW_MARKER		MAKE_MARKER ('A', 'L', 'a', 'w')
#define SOUN_MARKER		MAKE_MARKER ('S', 'o', 'u', 'n')
#define DFIL_MARKER		MAKE_MARKER ('d', 'F', 'i', 'l')
#define ESSN_MARKER		MAKE_MARKER ('e', '*', '*', '\0')
#define PSION_VERSION		((unsigned short) 3856)

/*------------------------------------------------------------------------------
** Private static functions.
*/

static int	wve_read_header (SF_PRIVATE *psf) ;
static int	wve_write_header (SF_PRIVATE *psf, int calc_length) ;
static int	wve_close (SF_PRIVATE *psf) ;

/*------------------------------------------------------------------------------
** Public function.
*/

int
wve_open (SF_PRIVATE *psf)
{	int	error = 0 ;

	if (psf->is_pipe)
		return SFE_WVE_NO_PIPE ;

	if (psf->mode == SFM_READ || (psf->mode == SFM_RDWR && psf->filelength > 0))
	{	if ((error = wve_read_header (psf)))
			return error ;
		} ;

	if (psf->mode == SFM_WRITE || psf->mode == SFM_RDWR)
	{	if ((psf->sf.format & SF_FORMAT_TYPEMASK) != SF_FORMAT_WVE)
			return	SFE_BAD_OPEN_FORMAT ;

		psf->endian = SF_ENDIAN_BIG ;

		if ((error = wve_write_header (psf, SF_FALSE)))
			return error ;

		psf->write_header = wve_write_header ;
		} ;

	psf->blockwidth = psf->bytewidth * psf->sf.channels ;

	psf->container_close = wve_close ;

	error = alaw_init (psf) ;

	return error ;
} /* wve_open */

/*------------------------------------------------------------------------------
*/

static int
wve_read_header (SF_PRIVATE *psf)
{	int marker ;
	unsigned short version, padding, repeats, trash ;
	unsigned datalength ;

	/* Set position to start of file to begin reading header. */
	psf_binheader_readf (psf, "pm", 0, &marker) ;
	if (marker != ALAW_MARKER)
		return SFE_WVE_NOT_WVE ;

	psf_binheader_readf (psf, "m", &marker) ;
	if (marker != SOUN_MARKER)
		return SFE_WVE_NOT_WVE ;

	psf_binheader_readf (psf, "m", &marker) ;
	if (marker != DFIL_MARKER)
		return SFE_WVE_NOT_WVE ;

	psf_binheader_readf (psf, "m", &marker) ;
	if (marker != ESSN_MARKER)
		return SFE_WVE_NOT_WVE ;

	psf_binheader_readf (psf, "E2", &version) ;
	if (version != PSION_VERSION)
        	return SFE_WVE_NOT_WVE ;

	psf_binheader_readf (psf, "E4", &datalength) ;
	psf->datalength = datalength ;
	psf->dataoffset = 0x20 ;
	if (psf->datalength != psf->filelength - psf->dataoffset)
		return SFE_WVE_NOT_WVE ;

	psf_binheader_readf (psf, "E2E2222", &padding, &repeats, &trash, &trash, &trash) ;

	psf_log_printf (psf, "Psion Palmtop Alaw (.wve)\n"
			"  Sample Rate : 8000\n"
			"  Channels    : 1\n"
			"  Encoding    : A-law\n") ;

	psf->sf.format		= SF_FORMAT_WVE | SF_FORMAT_ALAW ;
	psf->sf.samplerate	= 8000 ;
	psf->sf.frames		= psf->datalength ;
	psf->sf.channels	= 1 ;

	return SFE_NO_ERROR ;
} /* wve_read_header */

/*------------------------------------------------------------------------------
*/

static int
wve_write_header (SF_PRIVATE *psf, int calc_length)
{	sf_count_t	current ;
	unsigned short	padding = 0, repeats = 0 ;

	current = psf_ftell (psf) ;

	if (calc_length)
	{	psf->filelength = psf_get_filelen (psf) ;

		psf->datalength = psf->filelength - psf->dataoffset ;
		if (psf->dataend)
			psf->datalength -= psf->filelength - psf->dataend ;

		psf->sf.frames = psf->datalength / (psf->bytewidth * psf->sf.channels) ;
		} ;

	/* Reset the current header length to zero. */
	psf->header [0] = 0 ;
	psf->headindex = 0 ;
	psf_fseek (psf, 0, SEEK_SET) ;

	/* Write header. */
	psf_binheader_writef (psf, "Emmmm2422222", ALAW_MARKER, SOUN_MARKER, DFIL_MARKER, ESSN_MARKER, PSION_VERSION, psf->datalength, padding, repeats, 0, 0, 0) ;
	psf_fwrite (psf->header, psf->headindex, 1, psf) ;

	if (psf->sf.channels != 1)
		return SFE_CHANNEL_COUNT ;

	if (psf->error)
		return psf->error ;

	psf->dataoffset = psf->headindex ;

	if (current > 0)
		psf_fseek (psf, current, SEEK_SET) ;

	return psf->error ;
} /* wve_write_header */

/*------------------------------------------------------------------------------
*/

static int
wve_close (SF_PRIVATE *psf)
{
	if (psf->mode == SFM_WRITE || psf->mode == SFM_RDWR)
	{	/*  Now we know for certain the length of the file we can re-write
		**	the header.
		*/
		wve_write_header (psf, SF_TRUE) ;
		} ;

	return 0 ;
} /* wve_close */
