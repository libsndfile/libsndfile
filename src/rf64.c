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

/*
**	This format documented at:
**	http://www.sr.se/utveckling/tu/bwf/prog/RF_64v1_4.pdf
*/

#include	"sfconfig.h"

#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<ctype.h>

#include	"sndfile.h"
#include	"sfendian.h"
#include	"common.h"
#include	"wav_w64.h"

/*------------------------------------------------------------------------------
** Macros to handle big/little endian issues.
*/
#define	RF64_MARKER		MAKE_MARKER ('R', 'F', '6', '4')
#define	FFFF_MARKER		MAKE_MARKER (0xff, 0xff, 0xff, 0xff)
#define	WAVE_MARKER		MAKE_MARKER ('W', 'A', 'V', 'E')
#define	ds64_MARKER		MAKE_MARKER ('d', 's', '6', '4')
#define	fmt_MARKER		MAKE_MARKER ('f', 'm', 't', ' ')
#define	data_MARKER		MAKE_MARKER ('d', 'a', 't', 'a')


/*------------------------------------------------------------------------------
** Typedefs.
*/

typedef struct
{	
} RF64_PRIVATE ;

/*------------------------------------------------------------------------------
** Private static functions.
*/

static int	rf64_read_header (SF_PRIVATE *psf, int *blockalign, int *framesperblock) ;
static int	rf64_write_header (SF_PRIVATE *psf, int calc_length) ;
static int	rf64_close (SF_PRIVATE *psf) ;

/*------------------------------------------------------------------------------
** Public function.
*/

int
rf64_open (SF_PRIVATE *psf)
{	WAV_PRIVATE *wpriv ;
	int	subformat, error = 0, blockalign = 0, framesperblock = 0 ; ;

puts (__func__) ;

	if ((wpriv = calloc (1, sizeof (WAV_PRIVATE))) == NULL)
		return SFE_MALLOC_FAILED ;
	psf->container_data = wpriv ;

	if ((error = rf64_read_header (psf, &blockalign, &framesperblock)) != 0)
		return error ;

	if ((psf->sf.format & SF_FORMAT_TYPEMASK) != SF_FORMAT_RF64)
		return	SFE_BAD_OPEN_FORMAT ;

	subformat = psf->sf.format & SF_FORMAT_SUBMASK ;

	if (psf->mode == SFM_WRITE || psf->mode == SFM_RDWR)
	{	if (psf->is_pipe)
			return SFE_NO_PIPE_WRITE ;

		psf->endian = SF_ENDIAN_LITTLE ;		/* All RF64 files are little endian. */

		psf->blockwidth = psf->bytewidth * psf->sf.channels ;

		if (subformat == SF_FORMAT_IMA_ADPCM || subformat == SF_FORMAT_MS_ADPCM)
		{	blockalign = wav_w64_srate2blocksize (psf->sf.samplerate * psf->sf.channels) ;
			framesperblock = -1 ;

			/* FIXME : This block must go */
			psf->filelength = SF_COUNT_MAX ;
			psf->datalength = psf->filelength ;
			if (psf->sf.frames <= 0)
				psf->sf.frames = (psf->blockwidth) ? psf->filelength / psf->blockwidth : psf->filelength ;
			/* EMXIF : This block must go */
			} ;

		if ((error = rf64_write_header (psf, SF_FALSE)))
			return error ;

		psf->write_header = rf64_write_header ;
		} ;

	psf->container_close = rf64_close ;

	switch (subformat)
	{	case SF_FORMAT_PCM_U8 :
					error = pcm_init (psf) ;
					break ;

		case SF_FORMAT_PCM_16 :
		case SF_FORMAT_PCM_24 :
		case SF_FORMAT_PCM_32 :
					error = pcm_init (psf) ;
					break ;

		case SF_FORMAT_ULAW :
					error = ulaw_init (psf) ;
					break ;

		case SF_FORMAT_ALAW :
					error = alaw_init (psf) ;
					break ;

		/* Lite remove start */
		case SF_FORMAT_FLOAT :
					error = float32_init (psf) ;
					break ;

		case SF_FORMAT_DOUBLE :
					error = double64_init (psf) ;
					break ;

		case SF_FORMAT_IMA_ADPCM :
					error = wav_w64_ima_init (psf, blockalign, framesperblock) ;
					break ;

		case SF_FORMAT_MS_ADPCM :
					error = wav_w64_msadpcm_init (psf, blockalign, framesperblock) ;
					break ;
		/* Lite remove end */

		case SF_FORMAT_GSM610 :
					error = gsm610_init (psf) ;
					break ;

		default : 	return SFE_UNIMPLEMENTED ;
		} ;

	return error ;
} /* rf64_open */

/*------------------------------------------------------------------------------
*/

static int
rf64_read_header (SF_PRIVATE *psf, int *blockalign, int *framesperblock)
{	WAV_PRIVATE *wpriv ;
	WAV_FMT 	*wav_fmt ;
	sf_count_t riff_size, data_size ;
	unsigned int size32 ;
	int marker [3], error ;

	if ((wpriv = psf->container_data) == NULL)
		return SFE_INTERNAL ;

	/* Set position to start of file to begin reading header. */
	psf_binheader_readf (psf, "pmmm", 0, marker, marker + 1, marker + 2) ;
	if (marker [0] != RF64_MARKER || marker [1] != FFFF_MARKER || marker [2] != WAVE_MARKER)
		return SFE_WVE_NOT_WVE ;

	psf_log_printf (psf, "%M\n%M\n", RF64_MARKER, WAVE_MARKER) ;

	psf_binheader_readf (psf, "em4", marker, &size32) ;
	psf_log_printf (psf, "%M : %u\n", marker [0], size32) ;

	psf_binheader_readf (psf, "888", &riff_size, &data_size, &psf->sf.frames) ;
	psf_log_printf (psf, "  Riff size : %D\n  Data size : %D\n  Frames    : %D\n",
			riff_size, data_size, psf->sf.frames) ;

	psf_binheader_readf (psf, "4", &size32) ;
	psf_log_printf (psf, "  Table len : %u\n", size32) ;

	psf_binheader_readf (psf, "jm4", size32 + 4, marker, &size32) ;
	psf_log_printf (psf, "%M : %u\n", marker [0], size32) ;

	if ((error = wav_w64_read_fmt_chunk (psf, size32)) != 0)
		return error ;

	psf_binheader_readf (psf, "m4", marker, &size32) ;
	psf_log_printf (psf, "%M : %x\n", marker [0], size32) ;
	psf->dataoffset = psf->headindex ;

	wav_fmt = &wpriv->wav_fmt ;

	switch (wav_fmt->format)
	{	case WAVE_FORMAT_PCM :
		case WAVE_FORMAT_EXTENSIBLE :
					/* extensible might be FLOAT, MULAW, etc as well! */
					psf->sf.format = SF_FORMAT_RF64 | u_bitwidth_to_subformat (psf->bytewidth * 8) ;
					break ;

		case WAVE_FORMAT_MULAW :
					psf->sf.format = SF_FORMAT_RF64 | SF_FORMAT_ULAW ;
					break ;

		case WAVE_FORMAT_ALAW :
					psf->sf.format = SF_FORMAT_RF64 | SF_FORMAT_ALAW ;
					break ;

		case WAVE_FORMAT_MS_ADPCM :
					psf->sf.format = SF_FORMAT_RF64 | SF_FORMAT_MS_ADPCM ;
					*blockalign = wav_fmt->msadpcm.blockalign ;
					*framesperblock = wav_fmt->msadpcm.samplesperblock ;
					break ;

		case WAVE_FORMAT_IMA_ADPCM :
					psf->sf.format = SF_FORMAT_RF64 | SF_FORMAT_IMA_ADPCM ;
					*blockalign = wav_fmt->ima.blockalign ;
					*framesperblock = wav_fmt->ima.samplesperblock ;
					break ;

		case WAVE_FORMAT_GSM610 :
					psf->sf.format = SF_FORMAT_RF64 | SF_FORMAT_GSM610 ;
					break ;

		case WAVE_FORMAT_IEEE_FLOAT :
					psf->sf.format = SF_FORMAT_RF64 ;
					psf->sf.format |= (psf->bytewidth == 8) ? SF_FORMAT_DOUBLE : SF_FORMAT_FLOAT ;
					break ;

		default :
			psf_log_printf (psf, "UNIMPLEMENTED\n") ;
			return SFE_UNIMPLEMENTED ;
		} ;

	/* All RF64 files are little endian. */
	psf->endian = SF_ENDIAN_LITTLE ;

	psf_log_printf (psf, "End\n") ;

	return 0 ;
} /* rf64_read_header */


static int
rf64_write_header (SF_PRIVATE *psf, int calc_length)
{
	psf_log_printf (psf, "%s (%p, %d)\n", __func__, psf, calc_length) ;
	return 0 ;
} /* rf64_write_header */

static int
rf64_close (SF_PRIVATE *psf)
{
	psf_log_printf (psf, "%s (%p)\n", __func__, psf) ;
	return 0 ;
} /* rf64_close */
