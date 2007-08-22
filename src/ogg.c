/*
** Copyright (C) 2002-2007 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (C) 2007 John ffitch
**
** This program is free software ; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation ; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY ; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program ; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*
**  Much of this code is based on the examples in libvorbis from the
** XIPHOPHORUS Company http://www.xiph.org/ which has a BSD-style Licence
** Copyright (c) 2002, Xiph.org Foundation
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** - Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** - Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** - Neither the name of the Xiph.org Foundation nor the names of its
** contributors may be used to endorse or promote products derived from
** this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION
** OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES ; LOSS OF USE,
** DATA, OR PROFITS ; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "sfconfig.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "sndfile.h"
#include "sfendian.h"
#include "common.h"
#include "float_cast.h"

#include "VORBIS/include/vorbis/codec.h"
#include "VORBIS/include/vorbis/vorbisenc.h"

typedef int convert_func (int, void *, int, int, float **) ;

static int	ogg_read_header (SF_PRIVATE *psf, int log_data) ;
static int	ogg_write_init (SF_PRIVATE *psf, int calc_length) ;
static int	ogg_write_header (SF_PRIVATE *psf, int calc_length) ;
static int	ogg_close (SF_PRIVATE *psf) ;
static int	ogg_command (SF_PRIVATE *psf, int command, void *data, int datasize) ;
static sf_count_t	ogg_seek (SF_PRIVATE *psf, int mode, sf_count_t offset) ;
static sf_count_t	ogg_read_s (SF_PRIVATE *psf, short *ptr, sf_count_t len) ;
static sf_count_t	ogg_read_i (SF_PRIVATE *psf, int *ptr, sf_count_t len) ;
static sf_count_t	ogg_read_f (SF_PRIVATE *psf, float *ptr, sf_count_t len) ;
static sf_count_t	ogg_read_d (SF_PRIVATE *psf, double *ptr, sf_count_t len) ;
static sf_count_t	ogg_write_s (SF_PRIVATE *psf, const short *ptr, sf_count_t len) ;
static sf_count_t	ogg_write_i (SF_PRIVATE *psf, const int *ptr, sf_count_t len) ;
static sf_count_t	ogg_write_f (SF_PRIVATE *psf, const float *ptr, sf_count_t len) ;
static sf_count_t	ogg_write_d (SF_PRIVATE *psf, const double *ptr, sf_count_t len) ;
static sf_count_t	ogg_read_sample (SF_PRIVATE *psf, void *ptr, sf_count_t lens, convert_func *transfn) ;
static sf_count_t	ogg_length (SF_PRIVATE *psf) ;

typedef struct
{	int id ;
	const char *name ;
} STR_PAIRS ;

static STR_PAIRS vorbis_metatypes [] =
{	{	SF_STR_TITLE,		"Title" },
	{	SF_STR_COPYRIGHT,	"Copyright" },
	{	SF_STR_SOFTWARE,	"Software" },
	{	SF_STR_ARTIST,		"Artist" },
	{	SF_STR_COMMENT,		"Comment" },
	{	SF_STR_DATE,		"Date" },
	{	SF_STR_ALBUM,		"Album" },
	{	SF_STR_LICENSE,		"License" },
} ;

typedef struct
{	/* Sync and verify incoming physical bitstream */
	ogg_sync_state oy ;
	/* Take physical pages, weld into a logical stream of packets */
	ogg_stream_state os ;
	/* One Ogg bitstream page.  Vorbis packets are inside */
	ogg_page og ;
	/* One raw packet of data for decode */
	ogg_packet op ;
	int eos ;
} OGG_PRIVATE ;

typedef struct
{	/* Count current location */
	sf_count_t loc ;
	/* Struct that stores all the static vorbis bitstream settings */
	vorbis_info	vi ;
	/* Struct that stores all the bitstream user comments */
	vorbis_comment vc ;
	/* Ventral working state for the packet->PCM decoder */
	vorbis_dsp_state vd ;
	/* Local working space for packet->PCM decode */
	vorbis_block vb ;
} VORBIS_PRIVATE ;

static int
ogg_read_header (SF_PRIVATE *psf, int log_data)
{
	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	char *buffer ;
	int	 bytes ;
	int i, nn ;

	odata->eos = 0 ;

	/*
	**	Grab some data at the head of the stream.  We want the first page
	**	(which is guaranteed to be small and only contain the Vorbis
	**	stream initial header) We need the first page to get the stream
	**	serialno.
	*/

	/* Expose the buffer */
	buffer = ogg_sync_buffer (&odata->oy, 4096L) ;

	/* Grab the part of the header that has already been read. */
	memcpy (buffer, psf->header, psf->headindex) ;
	bytes = psf->headindex ;

	/* Submit a 4k block to libvorbis' Ogg layer */
	bytes += psf_fread (buffer + psf->headindex, 1, 4096 - psf->headindex, psf) ;
	ogg_sync_wrote (&odata->oy, bytes) ;

	/* Get the first page. */
	if ((nn = ogg_sync_pageout (&odata->oy, &odata->og)) != 1)
	{
		/* Have we simply run out of data?  If so, we're done. */
		if (bytes < 4096)
			return 0 ;

		/* Error case.  Must not be Vorbis data */
		psf_log_printf (psf, "Input does not appear to be an Ogg bitstream.\n") ;
		return SFE_MALFORMED_FILE ;
		} ;

	/*
	**	This function (off_read_header) gets called multiple times, so the OGG
	**	and vorbis structs have to be cleared every time we pass through to
	**	prevent memory leaks.
	*/
	vorbis_block_clear (&vdata->vb) ;
	vorbis_dsp_clear (&vdata->vd) ;
	vorbis_comment_clear (&vdata->vc) ;
	vorbis_info_clear (&vdata->vi) ;

	ogg_stream_clear (&odata->os) ;

	/*
	**	Get the serial number and set up the rest of decode.
	**	Serialno first ; use it to set up a logical stream.
	*/
	ogg_stream_init (&odata->os, ogg_page_serialno (&odata->og)) ;

	/*
	**	Extract the initial header from the first page and verify that the
	**	Ogg bitstream is in fact Vorbis data.
	**
	**	I handle the initial header first instead of just having the code
	**	read all three Vorbis headers at once because reading the initial
	**	header is an easy way to identify a Vorbis bitstream and it's
	**	useful to see that functionality seperated out.
	*/
	vorbis_info_init (&vdata->vi) ;
	vorbis_comment_init (&vdata->vc) ;

	if (ogg_stream_pagein (&odata->os, &odata->og) < 0)
	{	/* Error ; stream version mismatch perhaps. */
		psf_log_printf (psf, "Error reading first page of Ogg bitstream data\n") ;
		return SFE_MALFORMED_FILE ;
		} ;

	if (ogg_stream_packetout (&odata->os, &odata->op) != 1)
	{	/* No page? must not be vorbis. */
		psf_log_printf (psf, "Error reading initial header packet.\n") ;
		return SFE_MALFORMED_FILE ;
		} ;

	if (vorbis_synthesis_headerin (&vdata->vi, &vdata->vc, &odata->op) < 0)
	{	/* Error case ; not a vorbis header. */
		psf_log_printf (psf, "This Ogg bitstream does not contain Vorbis audio data.\n") ;
		return SFE_MALFORMED_FILE ;
		} ;

	/*
	**	Common Ogg metadata fields?
	**	TITLE, VERSION, ALBUM, TRACKNUMBER, ARTIST, PERFORMER, COPYRIGHT, LICENSE,
	**	ORGANIZATION, DESCRIPTION, GENRE, DATE, LOCATION, CONTACT, ISRC,
	*/

	if (log_data)
	{	int k ;

		for (k = 0 ; k < ARRAY_LEN (vorbis_metatypes) ; k++)
		{	char *dd ;

			dd = vorbis_comment_query (&vdata->vc, vorbis_metatypes [k].name, 0) ;
			if (dd == NULL)
				continue ;
			psf_store_string (psf, vorbis_metatypes [k].id, dd) ;
			} ;
		} ;

	/*
	**	At this point, we're sure we're Vorbis.	We've set up the logical (Ogg)
	**	bitstream decoder. Get the comment and codebook headers and set up the
	**	Vorbis decoder.
	**
	**	The next two packets in order are the comment and codebook headers.
	**	They're likely large and may span multiple pages.  Thus we reead
	**	and submit data until we get our two pacakets, watching that no
	**	pages are missing.  If a page is missing, error out ; losing a
	**	header page is the only place where missing data is fatal.
	*/

	i = 0 ;			 /* Count of number of packets read */
	while (i < 2)
	{	int result = ogg_sync_pageout (&odata->oy, &odata->og) ;
		if (result == 0)
		{	/* Need more data */
			buffer = ogg_sync_buffer (&odata->oy, 4096) ;
			bytes = psf_fread (buffer, 1, 4096, psf) ;

			if (bytes == 0 && i < 2)
			{	psf_log_printf (psf, "End of file before finding all Vorbis headers!\n") ;
				return SFE_MALFORMED_FILE ;
				} ;
			nn = ogg_sync_wrote (&odata->oy, bytes) ;
			}
		else if (result == 1)
		{	/*
			**	Don't complain about missing or corrupt data yet. We'll
			**	catch it at the packet output phase.
			**
			**	We can ignore any errors here as they'll also become apparent
			**	at packetout.
			*/
			nn = ogg_stream_pagein (&odata->os, &odata->og) ;
			while (i < 2)
			{	result = ogg_stream_packetout (&odata->os, &odata->op) ;
				if (result == 0)
					break ;
				if (result < 0)
				{	/*	Uh oh ; data at some point was corrupted or missing!
					**	We can't tolerate that in a header. Die. */
					psf_log_printf (psf, "Corrupt secondary header.	Exiting.\n") ;
					return SFE_MALFORMED_FILE ;
					} ;

				vorbis_synthesis_headerin (&vdata->vi, &vdata->vc, &odata->op) ;
				i++ ;
				} ;
			} ;
		} ;

	if (log_data)
	{	int k ;

		psf_log_printf (psf, "\nBitstream is %d channel, %D Hz\n", vdata->vi.channels, vdata->vi.rate) ;
		psf_log_printf (psf, "Encoded by: %s\n", vdata->vc.vendor) ;

		/* Throw the comments plus a few lines about the bitstream we're decoding. */
		psf_log_printf (psf, "Metadata :\n") ;

		for (k = 0 ; k < ARRAY_LEN (vorbis_metatypes) ; k++)
		{	char *dd ;

			dd = vorbis_comment_query (&vdata->vc, vorbis_metatypes [k].name, 0) ;
			if (dd == NULL)
				continue ;
			psf_store_string (psf, vorbis_metatypes [k].id, dd) ;
			psf_log_printf (psf, "  %-10s : %s\n", vorbis_metatypes [k].name, dd) ;
			} ;

		psf_log_printf (psf, "End\n") ;
		} ;

	psf->sf.samplerate	= vdata->vi.rate ;
	psf->sf.channels	= vdata->vi.channels ;
	psf->sf.format		= SF_FORMAT_OGG | SF_FORMAT_VORBIS ;

	/*	OK, got and parsed all three headers. Initialize the Vorbis
	**	packet->PCM decoder.
	**	Central decode state. */
	vorbis_synthesis_init (&vdata->vd, &vdata->vi) ;

	/*	Local state for most of the decode so multiple block decodes can
	**	proceed in parallel. We could init multiple vorbis_block structures
	**	for vd here. */
	vorbis_block_init (&vdata->vd, &vdata->vb) ;

	vdata->loc = 0 ;

	return 0 ;
} /* ogg_read_header */

static int
ogg_write_init (SF_PRIVATE *psf, int UNUSED (calc_length))
{
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	int ret ;

	vorbis_info_init (&vdata->vi) ;

	/* The style of encoding should be selectable here, VBR quality mode. */
	ret = vorbis_encode_init_vbr (&vdata->vi, psf->sf.channels, psf->sf.samplerate, 0.4f) ;

#if 0
	ret = vorbis_encode_init (&vdata->vi, psf->sf.channels, psf->sf.samplerate, -1, 128000, -1) ; /* average bitrate mode */
	ret = ( vorbis_encode_setup_managed (&vdata->vi, psf->sf.channels,
						 psf->sf.samplerate, -1, 128000, -1) ||
		vorbis_encode_ctl (&vdata->vi, OV_ECTL_RATEMANAGE_AVG, NULL) ||
		vorbis_encode_setup_init (&vdata->vi)) ;
#endif
	if (ret)
		return SFE_BAD_OPEN_FORMAT ;

	vdata->loc = 0 ;

	return 0 ;
} /* ogg_write_init */

static int
ogg_write_header (SF_PRIVATE *psf, int UNUSED (calc_length))
{
	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	int k ;

	/* add a comment */
	vorbis_comment_init (&vdata->vc) ;

	vorbis_comment_add_tag (&vdata->vc, "ENCODER", "libsndfile") ;
	for (k = 0 ; k < SF_MAX_STRINGS ; k++)
	{	const char * name ;

		if (psf->strings [k].type == 0)
			break ;

		switch (psf->strings [k].type)
		{	case SF_STR_TITLE :		name = "TITLE" ; break ;
			case SF_STR_COPYRIGHT : name = "COPYRIGHT" ; break ;
			case SF_STR_SOFTWARE :	name = "SOFTWARE" ; break ;
			case SF_STR_ARTIST :	name = "ARTIST" ; break ;
			case SF_STR_COMMENT :	name = "COMMENT" ; break ;
			case SF_STR_DATE :		name = "DATE" ; break ;
			case SF_STR_ALBUM :		name = "ALBUM" ; break ;
			case SF_STR_LICENSE :	name = "LICENSE" ; break ;
			default : continue ;
			} ;

		vorbis_comment_add_tag (&vdata->vc, name, psf->strings [k].str) ;
		} ;

	/* set up the analysis state and auxiliary encoding storage */
	vorbis_analysis_init (&vdata->vd, &vdata->vi) ;
	vorbis_block_init (&vdata->vd, &vdata->vb) ;

	/*
	**	Set up our packet->stream encoder.
	**	Pick a random serial number ; that way we can more likely build
	**	chained streams just by concatenation.
	*/

	ogg_stream_init (&odata->os, psf_rand_int32 ()) ;

	/* Vorbis streams begin with three headers ; the initial header (with
	   most of the codec setup parameters) which is mandated by the Ogg
	   bitstream spec.  The second header holds any comment fields.	 The
	   third header holds the bitstream codebook.  We merely need to
	   make the headers, then pass them to libvorbis one at a time ;
	   libvorbis handles the additional Ogg bitstream constraints */

	{	ogg_packet header ;
		ogg_packet header_comm ;
		ogg_packet header_code ;
		int result ;

		vorbis_analysis_headerout (&vdata->vd, &vdata->vc, &header, &header_comm, &header_code) ;
		ogg_stream_packetin (&odata->os, &header) ; /* automatically placed in its own page */
		ogg_stream_packetin (&odata->os, &header_comm) ;
		ogg_stream_packetin (&odata->os, &header_code) ;

		/* This ensures the actual
		 * audio data will start on a new page, as per spec
		 */
		while ((result = ogg_stream_flush (&odata->os, &odata->og)) != 0)
		{	psf_fwrite (odata->og.header, 1, odata->og.header_len, psf) ;
			psf_fwrite (odata->og.body, 1, odata->og.body_len, psf) ;
			} ;
	}

	return 0 ;
} /* ogg_write_header */

static int
ogg_close (SF_PRIVATE *psf)
{
	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;

	if (odata == NULL || vdata == NULL)
		return 0 ;

	/*	Clean up this logical bitstream ; before exit we shuld see if we're
	**	followed by another [chained]. */

	if (psf->mode == SFM_WRITE)
	{	vorbis_analysis_wrote (&vdata->vd, 0) ;
		while (vorbis_analysis_blockout (&vdata->vd, &vdata->vb) == 1)
		{

		/* analysis, assume we want to use bitrate management */
			vorbis_analysis (&vdata->vb, NULL) ;
			vorbis_bitrate_addblock (&vdata->vb) ;

			while (vorbis_bitrate_flushpacket (&vdata->vd, &odata->op))
			{	/* weld the packet into the bitstream */
				ogg_stream_packetin (&odata->os, &odata->op) ;

				/* write out pages (if any) */
				while (!odata->eos)
				{	int result = ogg_stream_pageout (&odata->os, &odata->og) ;
					if (result == 0) break ;
					psf_fwrite (odata->og.header, 1, odata->og.header_len, psf) ;
					psf_fwrite (odata->og.body, 1, odata->og.body_len, psf) ;

		/* this could be set above, but for illustrative purposes, I do
		   it here (to show that vorbis does know where the stream ends) */

					if (ogg_page_eos (&odata->og)) odata->eos = 1 ;
				}
			}
		}
	}

	ogg_stream_clear (&odata->os) ;

	/* ogg_page and ogg_packet structs always point to storage in
	   libvorbis.  They are never freed or manipulated directly */

	vorbis_block_clear (&vdata->vb) ;
	vorbis_dsp_clear (&vdata->vd) ;
	vorbis_comment_clear (&vdata->vc) ;
	vorbis_info_clear (&vdata->vi) ;	 /* must be called last */
	/* should look here to reopen if chained */

	/* OK, clean up the framer */
	ogg_sync_clear (&odata->oy) ;

	return 0 ;
} /* ogg_close */

int
ogg_open (SF_PRIVATE *psf)
{	OGG_PRIVATE* odata = calloc (1, sizeof (OGG_PRIVATE)) ;
	VORBIS_PRIVATE* vdata = calloc (1, sizeof (VORBIS_PRIVATE)) ;
	int	subformat, error = 0 ;

	psf->container_data = odata ;
	psf->codec_data = vdata ;

	if (psf->mode == SFM_RDWR)
		return SFE_BAD_MODE_RW ;

	if (psf->mode == SFM_WRITE && psf->sf.samplerate <= 22050)
	{
#if (defined (__amd64__) || defined (__ppc__))
		psf_log_printf (psf, "Known Vorbis encoder bug for samplerates <= 22050.\n") ;
		psf_log_printf (psf, "See : https://trac.xiph.org/ticket/1229\n") ;
		return SFE_VORBIS_ENCODER_BUG ;
#endif
		} ;

	if (psf->mode == SFM_READ)
	{	/* Call this here so it only gets called once, so no memory is leaked. */
		ogg_sync_init (&odata->oy) ;

		if ((error = ogg_read_header (psf, 1)))
			return error ;

		psf->read_short		= ogg_read_s ;
		psf->read_int		= ogg_read_i ;
		psf->read_float		= ogg_read_f ;
		psf->read_double	= ogg_read_d ;
		psf->sf.frames		= ogg_length (psf) ;
		} ;

	if ((psf->sf.format & SF_FORMAT_TYPEMASK) != SF_FORMAT_OGG)
	{	if ((error = ogg_write_header (psf, 0)))
			psf->write_header = ogg_write_header ;
		} ;
	subformat = psf->sf.format & SF_FORMAT_SUBMASK ;
	psf->container_close = ogg_close ;
	if (psf->mode == SFM_WRITE)
	{
		if ((error = ogg_write_init (psf, 0)))
			return error ;
		psf->write_header = ogg_write_header ;
		psf->write_short	= ogg_write_s ;
		psf->write_int		= ogg_write_i ;
		psf->write_float	= ogg_write_f ;
		psf->write_double	= ogg_write_d ;
		psf->sf.frames = SF_COUNT_MAX ; /* Unknown really */
		psf->str_flags = SF_STR_ALLOW_START ;
		} ;

	psf->bytewidth = 1 ;
	psf->blockwidth = psf->bytewidth * psf->sf.channels ;

	psf->seek = ogg_seek ;
	psf->command = ogg_command ;

	/* FIXME, FIXME, FIXME : Hack these here for now and correct later. */
	psf->sf.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS ;
	psf->sf.sections = 1 ;

	psf->datalength = 1 ;
	psf->dataoffset = 0 ;
	/* End FIXME. */

	return error ;
} /* ogg_open */

static int
ogg_command (SF_PRIVATE *UNUSED (psf), int command,
		 void *UNUSED (data), int UNUSED (datasize))
{
	switch (command)
	{
		default :
			return 0 ;
		} ;

	return 0 ;
} /* ogg_command */

static int
ogg_rnull (int samples, void *UNUSED (vptr), int UNUSED (off) , int channels, float **UNUSED (pcm))
{
	return samples * channels ;
} /* ogg_rnull */

static int
ogg_rshort (int samples, void *vptr, int off, int channels, float **pcm)
{
	short *ptr = (short*) vptr + off ;
	int i = 0, j, n ;
	for (j = 0 ; j < samples ; j++)
		for (n = 0 ; n < channels ; n++)
			ptr [i++] = lrintf (pcm [n][j] * 32767.0f) ;
	return i ;
} /* ogg_rshort */

static int
ogg_rint (int samples, void *vptr, int off, int channels, float **pcm)
{
	int *ptr = (int*) vptr + off ;
	int i = 0, j, n ;

	for (j = 0 ; j < samples ; j++)
		for (n = 0 ; n < channels ; n++)
			ptr [i++] = lrintf (pcm [n][j] * 2147483647.0f) ;
	return i ;
} /* ogg_rint */

static int
ogg_rfloat (int samples, void *vptr, int off, int channels, float **pcm)
{
	float *ptr = (float*) vptr + off ;
	int i = 0, j, n ;
	for (j = 0 ; j < samples ; j++)
		for (n = 0 ; n < channels ; n++)
			ptr [i++] = pcm [n][j] ;
	return i ;
} /* ogg_rfloat */

static int
ogg_rdouble (int samples, void *vptr, int off, int channels, float **pcm)
{
	double *ptr = (double*) vptr + off ;
	int i = 0, j, n ;
	for (j = 0 ; j < samples ; j++)
		for (n = 0 ; n < channels ; n++)
			ptr [i++] = pcm [n][j] ;
	return i ;
} /* ogg_rdouble */


static sf_count_t
ogg_read_sample (SF_PRIVATE *psf, void *ptr, sf_count_t lens, convert_func *transfn)
{
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	int len, samples, i = 0 ;
	float **pcm ;

	len = lens / psf->sf.channels ;

	while ((samples = vorbis_synthesis_pcmout (&vdata->vd, &pcm)) > 0)
	{	if (samples > len) samples = len ;
		i += transfn (samples, ptr, i, psf->sf.channels, pcm) ;
		len -= samples ;
		/* tell libvorbis how many samples we actually consumed */
		vorbis_synthesis_read (&vdata->vd, samples) ;
		vdata->loc += samples ;
		if (len == 0)
			return i ; /* Is this necessary */
	}
	goto start0 ;		 /* Jump into the nasty nest */
	while (len > 0 && !odata->eos)
	{
		while (len > 0 && !odata->eos)
		{	int result = ogg_sync_pageout (&odata->oy, &odata->og) ;
			if (result == 0) break ; /* need more data */
			if (result<0)
			{	/* missing or corrupt data at this page position */
				psf_log_printf (psf, "Corrupt or missing data in bitstream ; continuing...\n") ;
				}
			else
			{	/* can safely ignore errors at this point */
				ogg_stream_pagein (&odata->os, &odata->og) ;
			start0:
				while (1)
				{	result = ogg_stream_packetout (&odata->os, &odata->op) ;
					if (result == 0)
						break ; /* need more data */
					if (result < 0)
					{	/* missing or corrupt data at this page position */
						/* no reason to complain ; already complained above */
						}
					else
					{	/* we have a packet.	Decode it */
						if (vorbis_synthesis (&vdata->vb, &odata->op) == 0) /* test for success! */
							vorbis_synthesis_blockin (&vdata->vd, &vdata->vb) ;
		  /*
		  **pcm is a multichannel float vector.	 In stereo, for
		  example, pcm [0] is left, and pcm [1] is right.	 samples is
		  the size of each channel.	 Convert the float values
		  (-1.<=range<=1.) to whatever PCM format and write it out */

						while ((samples = vorbis_synthesis_pcmout (&vdata->vd, &pcm)) > 0)
						{	if (samples>len) samples = len ;
							i += transfn (samples, ptr, i, psf->sf.channels, pcm) ;
							len -= samples ;
							/* tell libvorbis how many samples we actually consumed */
							vorbis_synthesis_read (&vdata->vd, samples) ;
							vdata->loc += samples ;
							if (len == 0)
								return i ; /* Is this necessary */
							} ;
					}
				}
				if (ogg_page_eos (&odata->og)) odata->eos = 1 ;
			}
		}
		if (!odata->eos)
		{	char *buffer ;
			int bytes ;
			buffer = ogg_sync_buffer (&odata->oy, 4096) ;
			bytes = psf_fread (buffer, 1, 4096, psf) ;
			ogg_sync_wrote (&odata->oy, bytes) ;
			if (bytes == 0) odata->eos = 1 ;
		}
	}
	return i ;
} /* ogg_read_sample */

static sf_count_t
ogg_read_s (SF_PRIVATE *psf, short *ptr, sf_count_t lens)
{	return ogg_read_sample (psf, (void*) ptr, lens, ogg_rshort) ;
} /* ogg_read_s */

static sf_count_t
ogg_read_i (SF_PRIVATE *psf, int *ptr, sf_count_t lens)
{	return ogg_read_sample (psf, (void*) ptr, lens, ogg_rint) ;
} /* ogg_read_i */

static sf_count_t
ogg_read_f (SF_PRIVATE *psf, float *ptr, sf_count_t lens)
{	return ogg_read_sample (psf, (void*) ptr, lens, ogg_rfloat) ;
} /* ogg_read_f */

static sf_count_t
ogg_read_d (SF_PRIVATE *psf, double *ptr, sf_count_t lens)
{	return ogg_read_sample (psf, (void*) ptr, lens, ogg_rdouble) ;
} /* ogg_read_d */

/*==============================================================================
*/

static void
ogg_write_samples (SF_PRIVATE *psf, OGG_PRIVATE *odata, VORBIS_PRIVATE *vdata, int in_frames)
{
	vorbis_analysis_wrote (&vdata->vd, in_frames) ;

	/*
	**	Vorbis does some data preanalysis, then divvies up blocks for
	**	more involved (potentially parallel) processing. Get a single
	**	block for encoding now.
	*/
	while (vorbis_analysis_blockout (&vdata->vd, &vdata->vb) == 1)
	{
		/* analysis, assume we want to use bitrate management */
		vorbis_analysis (&vdata->vb, NULL) ;
		vorbis_bitrate_addblock (&vdata->vb) ;

		while (vorbis_bitrate_flushpacket (&vdata->vd, &odata->op))
		{
			/* weld the packet into the bitstream */
			ogg_stream_packetin (&odata->os, &odata->op) ;

			/* write out pages (if any) */
			while (!odata->eos)
			{	int result = ogg_stream_pageout (&odata->os, &odata->og) ;
				if (result == 0)
					break ;
				psf_fwrite (odata->og.header, 1, odata->og.header_len, psf) ;
				psf_fwrite (odata->og.body, 1, odata->og.body_len, psf) ;

				/*	This could be set above, but for illustrative purposes, I do
				**	it here (to show that vorbis does know where the stream ends) */
				if (ogg_page_eos (&odata->og))
					odata->eos = 1 ;
				} ;
			} ;
		} ;

	vdata->loc += in_frames ;
} /* ogg_write_data */


static sf_count_t
ogg_write_s (SF_PRIVATE *psf, const short *ptr, sf_count_t lens)
{
	int i, m, j = 0 ;
	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	int in_frames = lens / psf->sf.channels ;
	float **buffer = vorbis_analysis_buffer (&vdata->vd, in_frames) ;
	for (i = 0 ; i < in_frames ; i++)
		for (m = 0 ; m < psf->sf.channels ; m++)
			buffer [m][i] = (float) (ptr [j++]) / 32767.0f ;

	ogg_write_samples (psf, odata, vdata, in_frames) ;

	return lens ;
} /* ogg_write_s */

static sf_count_t
ogg_write_i (SF_PRIVATE *psf, const int *ptr, sf_count_t lens)
{	int i, m, j = 0 ;
	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	int in_frames = lens / psf->sf.channels ;
	float **buffer = vorbis_analysis_buffer (&vdata->vd, in_frames) ;
	for (i = 0 ; i < in_frames ; i++)
		for (m = 0 ; m < psf->sf.channels ; m++)
			buffer [m][i] = (float) (ptr [j++]) / 2147483647.0f ;

	ogg_write_samples (psf, odata, vdata, in_frames) ;

	return lens ;
} /* ogg_write_i */

static sf_count_t
ogg_write_f (SF_PRIVATE *psf, const float *ptr, sf_count_t lens)
{	int i, m, j = 0 ;
	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	int in_frames = lens / psf->sf.channels ;
	float **buffer = vorbis_analysis_buffer (&vdata->vd, in_frames) ;
	for (i = 0 ; i < in_frames ; i++)
		for (m = 0 ; m < psf->sf.channels ; m++)
			buffer [m][i] = ptr [j++] ;

	ogg_write_samples (psf, odata, vdata, in_frames) ;

	return lens ;
} /* ogg_write_f */

static sf_count_t
ogg_write_d (SF_PRIVATE *psf, const double *ptr, sf_count_t lens)
{	int i, m, j = 0 ;
	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	int in_frames = lens / psf->sf.channels ;
	float **buffer = vorbis_analysis_buffer (&vdata->vd, in_frames) ;
	for (i = 0 ; i < in_frames ; i++)
		for (m = 0 ; m < psf->sf.channels ; m++)
			buffer [m][i] = (float) ptr [j++] ;

	ogg_write_samples (psf, odata, vdata, in_frames) ;

	return lens ;
} /* ogg_write_d */

static sf_count_t
ogg_seek (SF_PRIVATE *psf, int UNUSED (mode), sf_count_t offset)
{
	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;

	if (odata == NULL || vdata == NULL)
		return 0 ;

	if (offset < 0)
	{	psf->error = SFE_BAD_SEEK ;
		return ((sf_count_t) -1) ;
		} ;

	if (psf->mode == SFM_READ)
	{	sf_count_t target = offset - vdata->loc ;

		if (target < 0)
		{	/* 12 to allow for OggS bit */
			psf_fseek (psf, 12, SEEK_SET) ;
			ogg_read_header (psf, 0) ; /* Reset state */
			target = offset ;
			} ;

		while (target > 0)
		{	sf_count_t m = target > 4096 ? 4096 : target ;
			ogg_read_sample (psf, (void*) NULL, m, ogg_rnull) ;
			target -= m ;
			} ;

		return vdata->loc ;
		} ;

	return 0 ;
} /* ogg_seek */

static sf_count_t
ogg_length (SF_PRIVATE *psf)
{	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	sf_count_t length ;

	if (psf->sf.seekable == 0)
		return SF_COUNT_MAX ;

	while (odata->eos == 0)
		ogg_read_sample (psf, NULL, 4096, ogg_rnull) ;

	length = vdata->loc ;
	psf_fseek (psf, 12, SEEK_SET) ;
	ogg_read_header (psf, 0) ;

	return length ;
} /* ogg_length */

