/*
** Copyright (C) 2001-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (C) 2004 Paavo Jumppanen
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
** The sd2 support implemented in this file was partially sponsored
** (financially) by Paavo Jumppanen.
*/

/*
** Documentation on the Mac resource fork was obtained here :
** http://developer.apple.com/documentation/mac/MoreToolbox/MoreToolbox-99.html
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	"sndfile.h"
#include	"config.h"
#include	"sfendian.h"
#include	"common.h"

/*------------------------------------------------------------------------------
 * Markers.
*/

#define	Sd2f_MARKER			MAKE_MARKER ('S', 'd', '2', 'f')
#define	Sd2a_MARKER			MAKE_MARKER ('S', 'd', '2', 'a')
#define	ALCH_MARKER			MAKE_MARKER ('A', 'L', 'C', 'H')
#define STR_MARKER			MAKE_MARKER ('S', 'T', 'R', ' ')

typedef struct
{	unsigned char * rsrc_data ;
	int rsrc_len ;

	int data_offset, data_length ;
	int map_offset, map_length ;

	int type_count, type_offset ;
	int item_offset ;

	int str_index, str_count ;

	int string_offset ;

	/* All the above just to get these three. */
	int sample_size, sample_rate, channels ;
} SD2_RSRC ;

/*------------------------------------------------------------------------------
 * Private static functions.
*/

static int sd2_close	(SF_PRIVATE *psf) ;
static int sd2_write_rsrc_fork (SF_PRIVATE *psf, int calc_length) ;

static int sd2_parse_rsrc_fork (SF_PRIVATE *psf) ;
static int parse_str_rsrc (SF_PRIVATE *psf, SD2_RSRC * rsrc) ;

/*------------------------------------------------------------------------------
** Public functions.
*/

int
sd2_open (SF_PRIVATE *psf)
{	int saved_filedes, subformat, error = 0 ;

	/* SD2 is always big endian. */
	psf->endian = SF_ENDIAN_BIG ;

	if (psf->mode == SFM_READ || (psf->mode == SFM_RDWR && psf->filelength > 0))
	{	saved_filedes = psf->filedes ;
		psf->filedes = psf->rsrcdes ;

		error = sd2_parse_rsrc_fork (psf) ;

		psf->filedes = saved_filedes ;
		if (error)
			return error ;
		} ;

	if ((psf->sf.format & SF_FORMAT_TYPEMASK) != SF_FORMAT_SD2)
		return	SFE_BAD_OPEN_FORMAT ;

	subformat = psf->sf.format & SF_FORMAT_SUBMASK ;

	if (psf->mode == SFM_WRITE || psf->mode == SFM_RDWR)
	{	saved_filedes = psf->filedes ;
		psf->filedes = psf->rsrcdes ;

		error = sd2_write_rsrc_fork (psf, SF_FALSE) ;

		psf->filedes = saved_filedes ;
		if (error)
			return error ;

		psf->write_header = sd2_write_rsrc_fork ;
		} ;

	psf->close = sd2_close ;

	psf->blockwidth = psf->bytewidth * psf->sf.channels ;

	switch (subformat)
	{	case SF_FORMAT_PCM_S8 :	/* 8-bit linear PCM. */
		case SF_FORMAT_PCM_16 :	/* 16-bit linear PCM. */
		case SF_FORMAT_PCM_24 :	/* 24-bit linear PCM */
				error = pcm_init (psf) ;
				break ;

		default :	break ;
		} ;

	return error ;
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

static int
sd2_write_rsrc_fork (SF_PRIVATE *psf, int UNUSED (calc_length))
{
	psf_log_printf (psf, "Writing of SD2 files not implemented yet.\n", __func__) ;

	return SFE_UNIMPLEMENTED ;
} /* sd2_write_rsrc_fork */

/*------------------------------------------------------------------------------
*/

static inline int
read_char (const unsigned char * data, int offset)
{	return data [offset] ;
} /* read_char */

static inline int
read_short (const unsigned char * data, int offset)
{	return BES2H_SHORT (((const short *) (data + offset)) [0]) ;
} /* read_char */

static inline int
read_int (const unsigned char * data, int offset)
{	return BEI2H_INT (((const int *) (data + offset)) [0]) ;
} /* read_char */

static void
read_str (const unsigned char * data, int offset, char * buffer, int buffer_len)
{	int k ;

	memset (buffer, 0, buffer_len) ;

	for (k = 0 ; k < buffer_len - 1 ; k++)
	{	if (isprint (data [offset + k]) == 0)
			return ;
		buffer [k] = data [offset + k] ;
		} ;
	return ;
} /* read_str */

static int
sd2_parse_rsrc_fork (SF_PRIVATE *psf)
{	SD2_RSRC rsrc ;
	int k, marker, error = 0 ;

	memset (&rsrc, 0, sizeof (rsrc)) ;

	rsrc.rsrc_len = psf_get_filelen (psf) ;
	psf_log_printf (psf, "Resource length : %d (0x%04X)\n", rsrc.rsrc_len, rsrc.rsrc_len) ;

	if (rsrc.rsrc_len > SIGNED_SIZEOF (psf->header))
		rsrc.rsrc_data = calloc (1, rsrc.rsrc_len) ;
	else
		rsrc.rsrc_data = psf->header ;

	/* Read in the whole lot. */
	psf_fread (rsrc.rsrc_data, rsrc.rsrc_len, 1, psf) ;

	/* Reset the header storage because we have changed to the rsrcdes. */
	psf->headindex = psf->headend = rsrc.rsrc_len ;

	rsrc.data_offset = read_int (rsrc.rsrc_data, 0) ;
	rsrc.map_offset = read_int (rsrc.rsrc_data, 4) ;
	rsrc.data_length = read_int (rsrc.rsrc_data, 8) ;
	rsrc.map_length = read_int (rsrc.rsrc_data, 12) ;

	psf_log_printf (psf, "  data offset : 0x%04X\n  map  offset : 0x%04X\n"
				"  data length : 0x%04X\n  map  length : 0x%04X\n",
				rsrc.data_offset, rsrc.map_offset, rsrc.data_length, rsrc.map_length) ;

	if (rsrc.data_offset > rsrc.rsrc_len)
	{	psf_log_printf (psf, "Error : rsrc.data_offset > len\n") ;
		error = SFE_SD2_BAD_DATA_OFFSET ;
		goto parse_rsrc_fork_cleanup ;
		} ;

	if (rsrc.map_offset > rsrc.rsrc_len)
	{	psf_log_printf (psf, "Error : rsrc.map_offset > len\n") ;
		error = SFE_SD2_BAD_MAP_OFFSET ;
		goto parse_rsrc_fork_cleanup ;
		} ;

	if (rsrc.data_length > rsrc.rsrc_len)
	{	psf_log_printf (psf, "Error : rsrc.data_length > len\n") ;
		error = SFE_SD2_BAD_DATA_LENGTH ;
		goto parse_rsrc_fork_cleanup ;
		} ;

	if (rsrc.map_length > rsrc.rsrc_len)
	{	psf_log_printf (psf, "Error : rsrc.map_length > len\n") ;
		error = SFE_SD2_BAD_MAP_LENGTH ;
		goto parse_rsrc_fork_cleanup ;
		} ;

	if (rsrc.data_offset + rsrc.data_length != rsrc.map_offset || rsrc.map_offset + rsrc.map_length != rsrc.rsrc_len)
	{	psf_log_printf (psf, "Error : This does not look like a MacOSX resource fork.\n") ;
		error = SFE_SD2_BAD_RSRC ;
		goto parse_rsrc_fork_cleanup ;
		} ;

	rsrc.string_offset = rsrc.map_offset + read_short (psf->header, rsrc.map_offset + 26) ;
	if (rsrc.string_offset > rsrc.rsrc_len)
	{	psf_log_printf (psf, "Bad string offset (%d).\n", rsrc.string_offset) ;
		error = SFE_SD2_BAD_RSRC ;
		goto parse_rsrc_fork_cleanup ;
		} ;

	rsrc.type_offset = rsrc.map_offset + 30 ;

	rsrc.type_count = read_short (psf->header, rsrc.map_offset + 28) + 1 ;
	if (rsrc.type_count < 1)
	{	printf ("Bad type count.\n") ;
		error = SFE_SD2_BAD_RSRC ;
		goto parse_rsrc_fork_cleanup ;
		} ;

	rsrc.item_offset = rsrc.type_offset + rsrc.type_count * 8 ;
	if (rsrc.item_offset < 0 || rsrc.item_offset > rsrc.rsrc_len)
	{	psf_log_printf (psf, "Bad item offset (%d).\n", rsrc.item_offset) ;
		error = SFE_SD2_BAD_RSRC ;
		goto parse_rsrc_fork_cleanup ;
		} ;

	rsrc.str_index = -1 ;
	for (k = 0 ; k < rsrc.type_count ; k ++)
	{	marker = read_int (psf->header, rsrc.type_offset + k * 8) ;

		if (marker == STR_MARKER)
		{	rsrc.str_index = k ;
			rsrc.str_count = read_short (psf->header, rsrc.type_offset + k * 8 + 4) + 1 ;
			error = parse_str_rsrc (psf, &rsrc) ;
			goto parse_rsrc_fork_cleanup ;
			} ;
		} ;

	psf_log_printf (psf, "No 'STR ' resource.\n") ;
	error = SFE_SD2_BAD_RSRC ;

parse_rsrc_fork_cleanup :

	if ((void *) rsrc.rsrc_data < (void *) psf || (void *) rsrc.rsrc_data > (void *) (psf + 1))
		free (rsrc.rsrc_data) ;

	return error ;
} /* sd2_parse_rsrc_fork */

static int
parse_str_rsrc (SF_PRIVATE *psf, SD2_RSRC * rsrc)
{	char name [32], value [32] ;
	int k, str_offset, data_offset, data_len, rsrc_id ;

	psf_log_printf (psf, "Finding parameters :\n") ;

	str_offset = rsrc->string_offset ;
	for (k = 0 ; k < rsrc->str_count ; k++)
	{	int slen ;

		slen = read_char (rsrc->rsrc_data, str_offset) ;
		read_str (rsrc->rsrc_data, str_offset + 1, name, SF_MIN (SIGNED_SIZEOF (name), slen + 1)) ;
		str_offset += slen + 1 ;

		rsrc_id = read_short (rsrc->rsrc_data, rsrc->item_offset + k * 12) ;

		data_offset = rsrc->data_offset + read_int (rsrc->rsrc_data, rsrc->item_offset + k * 12 + 4) ;
		if (data_offset < 0 || data_offset > rsrc->rsrc_len)
		{	psf_log_printf (psf, "Bad data offset (%d)\n", data_offset) ;
			return SFE_SD2_BAD_DATA_OFFSET ;
			} ;

		data_len = read_int (rsrc->rsrc_data, data_offset) ;
		if (data_len < 0 || data_len > rsrc->rsrc_len)
		{	psf_log_printf (psf, "%s : Bad data length (%d).\n", __func__, data_len) ;
			return SFE_SD2_BAD_RSRC ;
			} ;

		slen = read_char (rsrc->rsrc_data, data_offset + 4) ;
		read_str (rsrc->rsrc_data, data_offset + 5, value, SF_MIN (SIGNED_SIZEOF (value), slen + 1)) ;

		psf_log_printf (psf, "  %-12s   0x%04x    %4d    %2d    %2d    '%s'\n", name, data_offset, rsrc_id, data_len, slen, value) ;

		if (strcmp (name, "sample-size") == 0 && rsrc->sample_size == 0)
			rsrc->sample_size = strtol (value, NULL, 10) ;
		else if (strcmp (name, "sample-rate") == 0 && rsrc->sample_rate == 0)
			rsrc->sample_rate = strtol (value, NULL, 10) ;
		else if (strcmp (name, "channels") == 0 && rsrc->channels == 0)
			rsrc->channels = strtol (value, NULL, 10) ;
		} ;

	if (rsrc->sample_rate < 0)
	{	psf_log_printf (psf, "Bad sample rate (%d)\n", rsrc->sample_rate) ;
		return SFE_SD2_BAD_RSRC ;
		} ;

	if (rsrc->channels < 0)
	{	psf_log_printf (psf, "Bad channel count (%d)\n", rsrc->channels) ;
		return SFE_SD2_BAD_RSRC ;
		} ;

	psf->sf.samplerate = rsrc->sample_rate ;
	psf->sf.channels = rsrc->channels ;
	psf->bytewidth = rsrc->sample_size ;
	psf->dataoffset = 0 ;

	switch (rsrc->sample_size)
	{	case 1 :
			psf->sf.format = SF_FORMAT_SD2 | SF_FORMAT_PCM_S8 ;
			break ;

		case 2 :
			psf->sf.format = SF_FORMAT_SD2 | SF_FORMAT_PCM_16 ;
			break ;

		case 3 :
			psf->sf.format = SF_FORMAT_SD2 | SF_FORMAT_PCM_24 ;
			break ;

		default :
			psf_log_printf (psf, "Bad sample size (%d)\n", rsrc->sample_size) ;
			return SFE_SD2_BAD_SAMPLE_SIZE ;
		} ;

	psf_log_printf (psf, "ok\n") ;

	return 0 ;
} /* parse_str_rsrc */

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: 1ee183e5-6b9f-4c2c-bd0a-24f35595cefc
*/
