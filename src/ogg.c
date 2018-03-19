/*
** Copyright (C) 2002-2016 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include "sfconfig.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#else
#include "sf_unistd.h"
#endif

#include "sndfile.h"
#include "sfendian.h"
#include "common.h"

#if HAVE_EXTERNAL_XIPH_LIBS

#include <ogg/ogg.h>

#include "ogg.h"

static int	ogg_close (SF_PRIVATE *psf) ;
static int	ogg_stream_classify (SF_PRIVATE *psf, OGG_PRIVATE * odata) ;
static int	ogg_page_classify (SF_PRIVATE * psf, const ogg_page * og) ;

int ogg_read_first_page (SF_PRIVATE *psf, OGG_PRIVATE *odata)
{	char *buffer ;
	int	bytes ;

	/*
	** The ogg standard requires that the first pages of a physical ogg
	** bitstream be only the first pages of each logical bitstream. These
	** pages MUST have the Beginning-Of-Stream bit set, and must contain
	** only the stream's relevant header. Currently we only load the first
	** page and check that it contains a codec we support as supporting
	** multiplexed streams (video+audio(en)+audio(fs)+subtitles, etc) is
	** beyond the scope of this library.
	*/

	/* Weird stuff happens if these aren't called. */
	ogg_stream_reset (&odata->ostream) ;
	ogg_sync_reset (&odata->osync) ;

	/* Expose the buffer */
	buffer = ogg_sync_buffer (&odata->osync, 4096L) ;

	/*
	** Grab some data. Beginning-of-stream Ogg pages are guarenteed to be
	** small. 4096 bytes ought to be enough.
	*/

	/* Avoid seeking if the file has just been opened. */
	if (psf_ftell (psf) == psf->header.indx)
	{	/* Grab the part of the header that has already been read. */
		memcpy (buffer, psf->header.ptr, psf->header.indx) ;
		bytes = psf->header.indx ;
		bytes += psf_fread (buffer + psf->header.indx, 1, 4096 - psf->header.indx, psf) ;
		}
	else
	{	if (psf_fseek (psf, 0, SEEK_SET) != 0)
			return SFE_NOT_SEEKABLE ;
		bytes = psf_fread (buffer, 1, 4096, psf) ;
		}

	ogg_sync_wrote (&odata->osync, bytes) ;

	/* Get the first page. Check for Beginning-of-stream bit */
	if (ogg_sync_pageout (&odata->osync, &odata->opage) != 1 ||
		ogg_page_bos (&odata->opage) == 0)
	{
		/* Have we simply run out of data?  If so, we're done. */
		if (bytes < 4096)
			return 0 ;

		/*
		** Error case. Either must not be an Ogg bitstream, or is in the
		** middle of a bitstream (live capture), or in the middle of a
		** bitstream and no complete page was in the buffer.
		*/

		psf_log_printf (psf, "Input does not appear to be the start of an Ogg bitstream.\n") ;
		return SFE_MALFORMED_FILE ;
		} ;

	/*
	**	Get the serial number and set up the rest of decode.
	**	Serialno first ; use it to set up a logical stream.
	*/
	ogg_stream_clear (&odata->ostream) ;
	ogg_stream_init (&odata->ostream, ogg_page_serialno (&odata->opage)) ;

	if (ogg_stream_pagein (&odata->ostream, &odata->opage) < 0)
	{	/* Error ; stream version mismatch perhaps. */
		psf_log_printf (psf, "Error reading first page of Ogg bitstream data\n") ;
		return SFE_MALFORMED_FILE ;
		} ;

	if (ogg_stream_packetout (&odata->ostream, &odata->opacket) != 1)
	{	/* No page? */
		psf_log_printf (psf, "Error reading initial header page packet.\n") ;
		return SFE_MALFORMED_FILE ;
		} ;

	return 0 ;
}

int
ogg_open (SF_PRIVATE *psf)
{	OGG_PRIVATE* odata = calloc (1, sizeof (OGG_PRIVATE)) ;
	sf_count_t pos = psf_ftell (psf) ;
	int	error = 0 ;

	psf->container_data = odata ;
	psf->container_close = ogg_close ;

	if (psf->file.mode == SFM_RDWR)
		return SFE_BAD_MODE_RW ;

	if (psf->file.mode == SFM_READ)
		if ((error = ogg_stream_classify (psf, odata)) != 0)
			return error ;

	if (SF_ENDIAN (psf->sf.format) != 0)
		return SFE_BAD_ENDIAN ;

	switch (psf->sf.format)
	{	case SF_FORMAT_OGG | SF_FORMAT_VORBIS :
			return ogg_vorbis_open (psf) ;

		case SF_FORMAT_OGGFLAC :
			/* Reset everything to an initial state. */
			ogg_sync_clear (&odata->osync) ;
			ogg_stream_clear (&odata->ostream) ;
			psf_fseek (psf, pos, SEEK_SET) ;
			free (psf->container_data) ;
			psf->container_data = NULL ;
			psf->container_close = NULL ;
			return flac_open (psf) ;

#if ENABLE_EXPERIMENTAL_CODE
		case SF_FORMAT_OGG | SF_FORMAT_SPEEX :
			return ogg_speex_open (psf) ;

		case SF_FORMAT_OGG | SF_FORMAT_PCM_16 :
		case SF_FORMAT_OGG | SF_FORMAT_PCM_24 :
			return ogg_pcm_open (psf) ;
#endif

		default :
			break ;
		} ;

	psf_log_printf (psf, "%s : bad psf->sf.format 0x%x.\n", __func__, psf->sf.format) ;
	return SFE_INTERNAL ;
} /* ogg_open */


static int
ogg_close (SF_PRIVATE *psf)
{	OGG_PRIVATE* odata = psf->container_data ;

	ogg_sync_clear (&odata->osync) ;
	ogg_stream_clear (&odata->ostream) ;

	return 0 ;
} /* ogg_close */

static int
ogg_stream_classify (SF_PRIVATE *psf, OGG_PRIVATE* odata)
{	int error ;

	/* Call this here so it only gets called once, so no memory is leaked. */
	ogg_sync_init (&odata->osync) ;

	/* Load the first page in the physical bitstream. */
	if ((error = ogg_read_first_page (psf, odata)) != 0)
		return error ;

	odata->codec = ogg_page_classify (psf, &odata->opage) ;

	switch (odata->codec)
	{	case OGG_VORBIS :
			psf->sf.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS ;
			return 0 ;

		case OGG_FLAC :
		case OGG_FLAC0 :
			psf->sf.format = SF_FORMAT_OGGFLAC ;
			return 0 ;

		case OGG_SPEEX :
			psf->sf.format = SF_FORMAT_OGG | SF_FORMAT_SPEEX ;
			return 0 ;

		case OGG_PCM :
			psf_log_printf (psf, "Detected Ogg/PCM data. This is not supported yet.\n") ;
			return SFE_UNIMPLEMENTED ;

		default :
			break ;
		} ;

	psf_log_printf (psf, "This Ogg bitstream contains some uknown data type.\n") ;
	return SFE_UNIMPLEMENTED ;
} /* ogg_stream_classify */

/*==============================================================================
*/

static struct
{	const char *str, *name ;
	int len, codec ;
} codec_lookup [] =
{	{	"Annodex",		"Annodex",	8, OGG_ANNODEX },
	{	"AnxData",		"AnxData",	7, OGG_ANXDATA },
	{	"\177FLAC",		"Flac1",	5, OGG_FLAC },
	{	"fLaC",			"Flac0",	4, OGG_FLAC0 },
	{	"PCM     ",		"PCM",		8, OGG_PCM },
	{	"Speex",		"Speex",	5, OGG_SPEEX },
	{	"\001vorbis",	"Vorbis",	7, OGG_VORBIS },
} ;

static int
ogg_page_classify (SF_PRIVATE * psf, const ogg_page * og)
{	int k, len ;

	for (k = 0 ; k < ARRAY_LEN (codec_lookup) ; k++)
	{	if (codec_lookup [k].len > og->body_len)
			continue ;

		if (memcmp (og->body, codec_lookup [k].str, codec_lookup [k].len) == 0)
		{	psf_log_printf (psf, "Ogg stream data : %s\n", codec_lookup [k].name) ;
			psf_log_printf (psf, "Stream serialno : %u\n", (uint32_t) ogg_page_serialno (og)) ;
			return codec_lookup [k].codec ;
			} ;
		} ;

	len = og->body_len < 8 ? og->body_len : 8 ;

	psf_log_printf (psf, "Ogg_stream data : '") ;
	for (k = 0 ; k < len ; k++)
		psf_log_printf (psf, "%c", isprint (og->body [k]) ? og->body [k] : '.') ;
	psf_log_printf (psf, "'     ") ;
	for (k = 0 ; k < len ; k++)
		psf_log_printf (psf, " %02x", og->body [k] & 0xff) ;
	psf_log_printf (psf, "\n") ;

	return 0 ;
} /* ogg_page_classify */

#else /* HAVE_EXTERNAL_XIPH_LIBS */

int
ogg_open	(SF_PRIVATE *psf)
{
	psf_log_printf (psf, "This version of libsndfile was compiled without Ogg/Vorbis support.\n") ;
	return SFE_UNIMPLEMENTED ;
} /* ogg_open */

#endif
