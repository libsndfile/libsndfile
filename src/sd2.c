/*
** Copyright (C) 2001-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
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

/*
** Documentation on the Mac resource fork was obtained here :
** http://developer.apple.com/documentation/mac/MoreToolbox/MoreToolbox-99.html
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"sndfile.h"
#include	"config.h"
#include	"sfendian.h"
#include	"common.h"


#if (0 && ENABLE_EXPERIMENTAL_CODE == 0)

int
sd2_open	(SF_PRIVATE *psf)
{	if (psf)
		return SFE_UNIMPLEMENTED ;
	return (psf && 0) ;
} /* sd2_open */

#else

/*------------------------------------------------------------------------------
 * Macros to handle big/little endian issues.
*/

#define	RSRC_START_MARKER	MAKE_MARKER (0, 0, 1, 0)
#define	Sd2f_MARKER			MAKE_MARKER ('S', 'd', '2', 'f')
#define	Sd2a_MARKER			MAKE_MARKER ('S', 'd', '2', 'a')
#define	ALCH_MARKER			MAKE_MARKER ('A', 'L', 'C', 'H')


#define	SFE_SD2_BAD_RSRC_MARKER		666
#define SFE_SD2_BAD_SD2_MARKER		667
#define SFE_SD2_BAD_RSRC			668

/*------------------------------------------------------------------------------
 * Typedefs for file chunks.
*/



/*------------------------------------------------------------------------------
 * Private static functions.
*/

static int	sd2_close	(SF_PRIVATE *psf) ;
static int	parse_rsrc_fork (SF_PRIVATE *psf) ;

/*------------------------------------------------------------------------------
** Public functions.
*/

static int
parse_rsrc_fork (SF_PRIVATE *psf)
{	unsigned char byte ;
	int marker, marker2 ;
	int rsrc_data_offset, rsrc_map_offset, rsrc_data_length, rsrc_map_length ;
	int rsrc_filelen ;
	
	rsrc_filelen = psf_get_filelen (psf) ;
	psf_log_printf (psf, "Resource length : %d (0x%X)\n", rsrc_filelen, rsrc_filelen) ;

	/* Reset the header storage because we have changed to the rsrcdes. */
	psf->headindex = psf->headend = 0 ;

	psf_binheader_readf (psf, "Ep4444", 0, &rsrc_data_offset, &rsrc_map_offset, &rsrc_data_length, &rsrc_map_length) ;
	psf_log_printf (psf,
			" data offset : 0x%X\n map  offset : 0x%X\n data length : 0x%X\n map  length : 0x%X\n",
			rsrc_data_offset, rsrc_map_offset, rsrc_data_length, rsrc_map_length) ;

	if (rsrc_data_offset + rsrc_data_length != rsrc_map_offset)
	{	psf_log_printf (psf, "rsrc_data_offset + rsrc_data_length != rsrc_map_offset\n") ;
		return SFE_SD2_BAD_RSRC ;
		} ;
	
	if (rsrc_map_offset + rsrc_map_length != rsrc_filelen)
	{	psf_log_printf (psf, "rsrc_map_offset + rsrc_map_length != rsrc_filelen\n") ;
		return SFE_SD2_BAD_RSRC ;
		} ;
	

	psf_binheader_readf (psf, "Epm", 0, &marker) ;
	
	if (marker != RSRC_START_MARKER)
	{	psf_log_printf (psf, "Bad Marker : %08X\n", marker) ;
		return SFE_SD2_BAD_RSRC_MARKER ;
		} ;

	psf_log_printf (psf, "Marker : %08X\n", marker) ;
	
	psf_binheader_readf (psf, "Ep1", 0x30, &byte) ;
	psf_binheader_readf (psf, "b", psf->u.cbuf, (int) byte) ;
	psf->u.cbuf [(int) (byte + 1)] = 0 ;
	psf_log_printf (psf, "Name (0x%X) : %s\n", byte, psf->u.cbuf) ;

	psf_binheader_readf (psf, "j44", 4, &marker, &marker2) ;
	if (marker != Sd2f_MARKER || (marker2 != Sd2a_MARKER && marker2 != ALCH_MARKER))
	{	psf_log_printf (psf, "Bad Markers : %M %M\n", marker, marker2) ;
		return SFE_SD2_BAD_SD2_MARKER ;
		} ;

	psf_log_printf (psf, "Markers : %M %M\n", marker, marker2) ;


psf_log_printf (psf, "position : %X\n", (int) psf_ftell (psf)) ;

	return 0 ;
} /* parse_rsrc_fork */

int
sd2_open	(SF_PRIVATE *psf)
{	int		marker, software, rsrc_offset, len, error ;
	int 	rsrc_data_offset, rsrc_map_offset, rsrc_data_length, rsrc_map_length ;
	char	slen ;
	float	srate ;

	if (psf->rsrcdes >= 0)
	{	int saved_filedes ;
	
		saved_filedes = psf->filedes ;
		psf->filedes = psf->rsrcdes ;

		error = parse_rsrc_fork (psf) ;

		psf->filedes = saved_filedes ;

		/*-if (error)
			return error ;-*/
		} ;

puts (psf->logbuffer) ;
exit (1) ;

	/* Read only so far. */

	psf_binheader_readf (psf, "Epmmj", 0x41, &marker, &software, 14) ;

	if (marker != Sd2f_MARKER)
	{	printf ("Whoops!!!\n") ;
		puts (psf->logbuffer) ;
		return SFE_INTERNAL ;
		} ;

	psf_log_printf (psf, "Marker   : %M\n"
						 "Software : %M\n",
			marker, software) ;

	/* This seems to be a constant for binhex files. */
	psf->dataoffset = 0x80 ;

	/* All SD2 files are big endian. */
	psf->endian= SF_ENDIAN_BIG ;

	/*
	**	Resource header info from:
	**	http://developer.apple.com/techpubs/mac/MoreToolbox/MoreToolbox-99.html
	*/

	rsrc_offset = psf->datalength + psf->dataoffset ;
	if (rsrc_offset & 0x7F)
		rsrc_offset = rsrc_offset - (rsrc_offset & 0x7F) + psf->dataoffset ;

	psf_log_printf (psf, "Resource offset : 0x%X\n", rsrc_offset) ;

	/* Jump to the rsrc_offset fork section. */
	psf_binheader_readf (psf, "Ep", rsrc_offset) ;

	psf_binheader_readf (psf, "E4444", &rsrc_data_offset, &rsrc_map_offset, &rsrc_data_length, &rsrc_map_length) ;

	rsrc_data_offset += rsrc_offset ;
	rsrc_map_offset	+= rsrc_offset ;

	psf_log_printf (psf, " data offset : 0x%X\n"
						 " map  offset : 0x%X\n"
						 " data length : 0x%X\n"
						 " map  length : 0x%X\n",

			rsrc_data_offset, rsrc_map_offset, rsrc_data_length, rsrc_map_length) ;

	if (rsrc_data_offset + rsrc_data_length > rsrc_map_offset || rsrc_map_offset + rsrc_map_length > psf->filelength)
	{	puts ("##############################") ;
		puts (psf->logbuffer) ;
		puts ("##############################") ;
		return SFE_INTERNAL ;
		} ;

	memset (psf->u.cbuf, 0, sizeof (psf->u.cbuf)) ;

	psf_binheader_readf (psf, "Ep41", rsrc_data_offset, &len, &slen) ;
	if (slen + 1 == len)
	{	psf_binheader_readf (psf, "Eb", psf->u.cbuf, len - 1) ;
		psf->u.cbuf [len - 1] = 0 ;
		if (sscanf (psf->u.cbuf, "%d", &len) == 1)
			psf->bytewidth = len ;
		} ;

	psf_binheader_readf (psf, "E41", &len, &slen) ;
	if (slen + 1 == len)
	{	psf_binheader_readf (psf, "Eb", psf->u.cbuf, len - 1) ;
		psf->u.cbuf [len - 1] = 0 ;
		if (sscanf (psf->u.cbuf, "%f", &srate) == 1)
			psf->sf.samplerate = srate ;
		} ;

	psf_binheader_readf (psf, "E41", &len, &slen) ;
	if (slen + 1 == len)
	{	psf_binheader_readf (psf, "Eb", psf->u.cbuf, len - 1) ;
		psf->u.cbuf [len - 1] = 0 ;
		if (sscanf (psf->u.cbuf, "%d", &len) == 1)
			psf->sf.channels = len ;
		} ;

	psf_log_printf (psf, "  byte width  : %d\n", psf->bytewidth) ;
	psf_log_printf (psf, "  sample rate : %d\n", psf->sf.samplerate) ;
	psf_log_printf (psf, "  channels    : %d\n", psf->sf.channels) ;

	if (psf->bytewidth == 2)
	{	psf->sf.format = SF_FORMAT_SD2 | SF_FORMAT_PCM_16 ;

		psf->blockwidth = psf->bytewidth * psf->sf.channels ;

		psf->sf.frames = psf->datalength / psf->blockwidth ;
		} ;

	pcm_init (psf) ;

	psf_fseek (psf, psf->dataoffset, SEEK_SET) ;

	psf->close = sd2_close ;

	return 0 ;
} /* sd2_open */

/*------------------------------------------------------------------------------
*/

static int
sd2_close	(SF_PRIVATE *psf)
{
	if (psf->mode == SFM_WRITE)
	{	/*  Now we know for certain the audio_length of the file we can re-write
		**	correct values for the FORM, 8SVX and BODY chunks.
		*/

		} ;

	return 0 ;
} /* sd2_close */

#endif

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 1ee183e5-6b9f-4c2c-bd0a-24f35595cefc
*/
