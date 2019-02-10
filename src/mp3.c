/*
** Copyright (C) 2019 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (C) 2019 Arthur Taylor <art@ified.ca>
**
** This program is free software ; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation ; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY ; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program ; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*
** What is an MP3 file anyways?
**
** Believe it or not, MP3 files don't exist.
**
** The MPEG-1 standard defined a few audio codecs. The standard only defined a
** streaming format of semi-independent frames of audio meant for broadcasting,
** with no details or hints about stored on-disk formats. Each frame defines
** it's own bitrate, channel count, sample rate. However, they aren't
** completely independent.
**
** With its amazing-for-the-time compression ratio, the layer III audio codec
** became quite popular with file sharers and the internet. A stream of layer
** III audio would simply be written as a file, usually with the extension
** .mp3. Over time enthusiast and proprietary encoders sprung up adding
** different metadata headers and trailers, file seeking tables, and fiddling
** with the codecs parameters. These changes are only really based on consensus.
**
** MPEG-1 I/II/III audio can be embedded in a few container formats (including
** WAV), stored raw, or with additional metadata extension headers and trailers.
**
** This file is concerned only with the most common case of MPEG Layer III
** audio without a container but with the additional metadata standards.
**
** For the purposes of libsndfile, the major format of SF_FORMAT_MP3 means the
** following assumptions. A file of major format type SF_FORMAT_MP3:
** - Contains only layer III audio frames (SF_FORMAT_MPEG_LAYER_III)
** - All MPEG frames contained in the file have the same channel count
** - All MPEG frames contained in the file have the same samplerate
** - Has at least one of:
**   - an ID3v1 trailer
**   - an ID3v2 header or trailer
**   - A Lame/Xing/Info header
**
** Testing has revealed that, more than any other format, MP3 suffers from
** corrupt files in the wild that most other software 'just works' with. This is
** usually because the MP3 decoders are very lenient. They are aided by the
** presence of a regular sync frame, but this makes it hard to classify them
** in a library that consumes other better-specified file formats.
*/

#include	"sfconfig.h"

#include	"sndfile.h"
#include	"common.h"

#if HAVE_MPEG

#include "mpeg.h"

static int	mp3_write_header (SF_PRIVATE *psf, int calc_length) ;
static int	mp3_command (SF_PRIVATE *psf, int command, void *data, int datasize) ;

/*------------------------------------------------------------------------------
 * Private functions
 */

static int
mp3_write_header (SF_PRIVATE *psf, int UNUSED (calc_length))
{
	if (psf->have_written)
		return 0 ;

	return mpeg_l3_encoder_write_id3tag (psf) ;
} ;

static int
mp3_command (SF_PRIVATE *psf, int command, void *data, int datasize)
{	int bitrate_mode ;

	switch (command)
	{	case SFC_SET_COMPRESSION_LEVEL :
			if (data == NULL || datasize != sizeof (double))
			{	psf->error = SFE_BAD_COMMAND_PARAM ;
				return SF_FALSE ;
				} ;
			if (psf->file.mode != SFM_WRITE)
			{	psf->error = SFE_NOT_WRITEMODE ;
				return SF_FALSE ;
				} ;
			return mpeg_l3_encoder_set_quality (psf, *(double *) data) ;

		case SFC_SET_BITRATE_MODE :
			if (psf->file.mode != SFM_WRITE)
			{	psf->error = SFE_NOT_WRITEMODE ;
				return SF_FALSE ;
				} ;
			if (data == NULL || datasize != sizeof (int))
			{	psf->error = SFE_BAD_COMMAND_PARAM ;
				return SF_FALSE ;
				} ;
			bitrate_mode = *(int *) data ;
			return mpeg_l3_encoder_set_bitrate_mode (psf, bitrate_mode) ;

		case SFC_GET_BITRATE_MODE :
			if (psf->file.mode == SFM_READ)
				return mpeg_decoder_get_bitrate_mode (psf) ;
			else
				return mpeg_l3_encoder_get_bitrate_mode (psf) ;

		default :
			return SF_FALSE ;
		} ;

	return SF_FALSE ;
} /* mpeg_command */

/*------------------------------------------------------------------------------
 * Public functions
 */

int
mp3_open (SF_PRIVATE *psf)
{	int error ;

	if (psf->file.mode == SFM_RDWR)
		return SFE_BAD_MODE_RW ;

	if (psf->file.mode == SFM_WRITE)
	{	if (SF_CODEC (psf->sf.format) != SF_FORMAT_MPEG_LAYER_III)
			return SFE_BAD_OPEN_FORMAT ;

		if ((error = mpeg_l3_encoder_init (psf, SF_TRUE)))
			return error ;

		/* Choose variable bitrate mode by default for standalone (mp3) files.*/
		mpeg_l3_encoder_set_bitrate_mode (psf, SF_BITRATE_MODE_VARIABLE) ;

		/* ID3 support */
		psf->strings.flags = SF_STR_ALLOW_START ;
		psf->write_header = mp3_write_header ;
		psf->datalength = 0 ;
		psf->dataoffset = 0 ;
		} ;

	if (psf->file.mode == SFM_READ)
	{	if ((error = mpeg_decoder_init (psf)))
			return error ;
		} ;

	psf->command = mp3_command ;

	return 0 ;
} /* mpeg_open */

#else /* HAVE_MPEG */

int
mp3_open (SF_PRIVATE *psf)
{
	psf_log_printf (psf, "This version of libsndfile was compiled without MP3 support.\n") ;
	return SFE_UNIMPLEMENTED ;
} /* mpeg_open */

#endif
