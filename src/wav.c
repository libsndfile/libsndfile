/*
** Copyright (C) 1999-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (C) 2004 David Viens <davidv@plogue.com>
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
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	"sndfile.h"
#include	"config.h"
#include	"sfendian.h"
#include	"common.h"
#include	"wav_w64.h"

/*------------------------------------------------------------------------------
 * Macros to handle big/little endian issues.
 */

#define RIFF_MARKER	 (MAKE_MARKER ('R', 'I', 'F', 'F'))
#define WAVE_MARKER	 (MAKE_MARKER ('W', 'A', 'V', 'E'))
#define fmt_MARKER	 (MAKE_MARKER ('f', 'm', 't', ' '))
#define data_MARKER	 (MAKE_MARKER ('d', 'a', 't', 'a'))
#define fact_MARKER	 (MAKE_MARKER ('f', 'a', 'c', 't'))
#define PEAK_MARKER	 (MAKE_MARKER ('P', 'E', 'A', 'K'))

#define cue_MARKER	 (MAKE_MARKER ('c', 'u', 'e', ' '))
#define LIST_MARKER	 (MAKE_MARKER ('L', 'I', 'S', 'T'))
#define slnt_MARKER	 (MAKE_MARKER ('s', 'l', 'n', 't'))
#define wavl_MARKER	 (MAKE_MARKER ('w', 'a', 'v', 'l'))
#define INFO_MARKER	 (MAKE_MARKER ('I', 'N', 'F', 'O'))
#define plst_MARKER	 (MAKE_MARKER ('p', 'l', 's', 't'))
#define adtl_MARKER	 (MAKE_MARKER ('a', 'd', 't', 'l'))
#define labl_MARKER	 (MAKE_MARKER ('l', 'a', 'b', 'l'))
#define ltxt_MARKER	 (MAKE_MARKER ('l', 't', 'x', 't'))
#define note_MARKER	 (MAKE_MARKER ('n', 'o', 't', 'e'))
#define smpl_MARKER	 (MAKE_MARKER ('s', 'm', 'p', 'l'))
#define bext_MARKER	 (MAKE_MARKER ('b', 'e', 'x', 't'))
#define MEXT_MARKER	 (MAKE_MARKER ('M', 'E', 'X', 'T'))
#define DISP_MARKER	 (MAKE_MARKER ('D', 'I', 'S', 'P'))
#define acid_MARKER	 (MAKE_MARKER ('a', 'c', 'i', 'd'))
#define strc_MARKER	 (MAKE_MARKER ('s', 't', 'r', 'c'))
#define PAD_MARKER	 (MAKE_MARKER ('P', 'A', 'D', ' '))
#define afsp_MARKER	 (MAKE_MARKER ('a', 'f', 's', 'p'))
#define clm_MARKER	 (MAKE_MARKER ('c', 'l', 'm', ' '))

#define ISFT_MARKER	 (MAKE_MARKER ('I', 'S', 'F', 'T'))
#define ICRD_MARKER	 (MAKE_MARKER ('I', 'C', 'R', 'D'))
#define ICOP_MARKER	 (MAKE_MARKER ('I', 'C', 'O', 'P'))
#define IARL_MARKER	 (MAKE_MARKER ('I', 'A', 'R', 'L'))
#define IART_MARKER	 (MAKE_MARKER ('I', 'A', 'R', 'T'))
#define INAM_MARKER	 (MAKE_MARKER ('I', 'N', 'A', 'M'))
#define IENG_MARKER	 (MAKE_MARKER ('I', 'E', 'N', 'G'))
#define IART_MARKER	 (MAKE_MARKER ('I', 'A', 'R', 'T'))
#define ICOP_MARKER	 (MAKE_MARKER ('I', 'C', 'O', 'P'))
#define IPRD_MARKER	 (MAKE_MARKER ('I', 'P', 'R', 'D'))
#define ISRC_MARKER	 (MAKE_MARKER ('I', 'S', 'R', 'C'))
#define ISBJ_MARKER	 (MAKE_MARKER ('I', 'S', 'B', 'J'))
#define ICMT_MARKER	 (MAKE_MARKER ('I', 'C', 'M', 'T'))

/* Weird WAVPACK marker which can show up at the start of the DATA section. */
#define wvpk_MARKER (MAKE_MARKER ('w', 'v', 'p', 'k'))
#define OggS_MARKER (MAKE_MARKER ('O', 'g', 'g', 'S'))

enum
{	HAVE_RIFF	= 0x01,
	HAVE_WAVE	= 0x02,
	HAVE_fmt	= 0x04,
	HAVE_fact	= 0x08,
	HAVE_PEAK	= 0x10,
	HAVE_data	= 0x20,
	HAVE_other	= 0x80000000
} ;



/*  known WAVEFORMATEXTENSIBLE GUIDS  */
static const EXT_SUBFORMAT MSGUID_SUBTYPE_PCM =
{	0x00000001, 0x0000, 0x0010, {	0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 }
} ;

static const EXT_SUBFORMAT MSGUID_SUBTYPE_MS_ADPCM =
{	0x00000002, 0x0000, 0x0010, {	0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 }
} ;

static const EXT_SUBFORMAT MSGUID_SUBTYPE_IEEE_FLOAT =
{	0x00000003, 0x0000, 0x0010, {	0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 }
} ;

static const EXT_SUBFORMAT MSGUID_SUBTYPE_ALAW =
{	0x00000006, 0x0000, 0x0010, {	0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 }
} ;

static const EXT_SUBFORMAT MSGUID_SUBTYPE_MULAW =
{	0x00000007, 0x0000, 0x0010, {	0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 }
} ;

#if 0
/* maybe interesting one day to read the following through sf_read_raw */
/* http://www.bath.ac.uk/~masrwd/pvocex/pvocex.html */
static const EXT_SUBFORMAT MSGUID_SUBTYPE_PVOCEX =
{	0x8312B9C2, 0x2E6E, 0x11d4, {	0xA8, 0x24, 0xDE, 0x5B, 0x96, 0xC3, 0xAB, 0x21 }
} ;
#endif

/*------------------------------------------------------------------------------
** Private static functions.
*/

static int	wav_read_header	 (SF_PRIVATE *psf, int *blockalign, int *framesperblock) ;
static int	wav_write_header (SF_PRIVATE *psf, int calc_length) ;

static void wavex_write_guid (SF_PRIVATE *psf, const EXT_SUBFORMAT * subformat) ;
static int	wavex_write_header (SF_PRIVATE *psf, int calc_length) ;

static int	wav_write_tailer (SF_PRIVATE *psf) ;
static void wav_write_strings (SF_PRIVATE *psf, int location) ;
static int	wav_command (SF_PRIVATE *psf, int command, void *data, int datasize) ;
static int	wav_close (SF_PRIVATE *psf) ;

static int 	wav_subchunk_parse	 (SF_PRIVATE *psf, int chunk) ;
static int	wav_read_smpl_chunk (SF_PRIVATE *psf, unsigned int chunklen) ;
static int	wav_read_acid_chunk (SF_PRIVATE *psf, unsigned int chunklen) ;

static int wavex_write_guid_equal (const EXT_SUBFORMAT * first, const EXT_SUBFORMAT * second) ;

/*------------------------------------------------------------------------------
** Public function.
*/

int
wav_open	 (SF_PRIVATE *psf)
{	int	format, subformat, error, blockalign = 0, framesperblock = 0 ;

	if (psf->mode == SFM_READ || (psf->mode == SFM_RDWR && psf->filelength > 0))
	{	if ((error = wav_read_header (psf, &blockalign, &framesperblock)))
			return error ;
		} ;

	subformat = psf->sf.format & SF_FORMAT_SUBMASK ;

	if (psf->mode == SFM_WRITE || psf->mode == SFM_RDWR)
	{	if (psf->is_pipe)
			return SFE_NO_PIPE_WRITE ;

		format = psf->sf.format & SF_FORMAT_TYPEMASK ;
		if (format != SF_FORMAT_WAV && format != SF_FORMAT_WAVEX)
			return	SFE_BAD_OPEN_FORMAT ;

		psf->endian = SF_ENDIAN_LITTLE ;	/* All WAV files are little endian. */

		psf->blockwidth = psf->bytewidth * psf->sf.channels ;

		if (psf->mode != SFM_RDWR || psf->filelength < 44)
		{	psf->filelength = 0 ;
			psf->datalength = 0 ;
			psf->dataoffset = 0 ;
			psf->sf.frames = 0 ;
			} ;

		if (subformat == SF_FORMAT_IMA_ADPCM || subformat == SF_FORMAT_MS_ADPCM)
		{	blockalign = wav_w64_srate2blocksize (psf->sf.samplerate * psf->sf.channels) ;
			framesperblock = -1 ; /* Corrected later. */
			} ;

		psf->str_flags = SF_STR_ALLOW_START | SF_STR_ALLOW_END ;

		/* By default, add the peak chunk to floating point files. Default behaviour
		** can be switched off using sf_command (SFC_SET_PEAK_CHUNK, SF_FALSE).
		*/
		if (psf->mode == SFM_WRITE && (subformat == SF_FORMAT_FLOAT || subformat == SF_FORMAT_DOUBLE))
		{	psf->pchunk = calloc (1, sizeof (PEAK_CHUNK) * psf->sf.channels * sizeof (PEAK_POS)) ;
			if (psf->pchunk == NULL)
				return SFE_MALLOC_FAILED ;
			psf->has_peak = SF_TRUE ;
			psf->peak_loc = SF_PEAK_START ;
			} ;

		psf->write_header = (format == SF_FORMAT_WAV) ? wav_write_header : wavex_write_header ;
		} ;

	psf->close = wav_close ;
	psf->command = wav_command ;

	switch (subformat)
	{	case SF_FORMAT_PCM_U8 :
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

	if (psf->mode == SFM_WRITE || (psf->mode == SFM_RDWR && psf->filelength == 0))
		return psf->write_header (psf, SF_FALSE) ;

	return error ;
} /* wav_open */

/*=========================================================================
** Private functions.
*/

static int
wav_read_header	 (SF_PRIVATE *psf, int *blockalign, int *framesperblock)
{	WAV_FMT		wav_fmt ;
	FACT_CHUNK	fact_chunk ;
	int			dword, marker, RIFFsize, done = 0 ;
	int			parsestage = 0, error, format = 0 ;
	char		*cptr ;

	/* Set position to start of file to begin reading header. */
	psf_binheader_readf (psf, "p", 0) ;

	while (! done)
	{	psf_binheader_readf (psf, "m", &marker) ;

		switch (marker)
		{	case RIFF_MARKER :
					if (parsestage)
						return SFE_WAV_NO_RIFF ;

					parsestage |= HAVE_RIFF ;

					psf_binheader_readf (psf, "e4", &RIFFsize) ;

					if (psf->fileoffset > 0 && psf->filelength > RIFFsize + 8)
					{	/* Set file length. */
						psf->filelength = RIFFsize + 8 ;
						psf_log_printf (psf, "RIFF : %u\n", RIFFsize) ;
						}
					else if (psf->filelength < RIFFsize + 2 * SIGNED_SIZEOF (dword))
					{	psf_log_printf (psf, "RIFF : %u (should be %D)\n", RIFFsize, psf->filelength - 2 * SIGNED_SIZEOF (dword)) ;
						RIFFsize = dword ;
						}
					else
						psf_log_printf (psf, "RIFF : %u\n", RIFFsize) ;

					break ;

			case WAVE_MARKER :
					if ((parsestage & HAVE_RIFF) != HAVE_RIFF)
						return SFE_WAV_NO_WAVE ;
					parsestage |= HAVE_WAVE ;

					psf_log_printf (psf, "WAVE\n") ;
					break ;

			case fmt_MARKER :
					if ((parsestage & (HAVE_RIFF | HAVE_WAVE)) != (HAVE_RIFF | HAVE_WAVE))
						return SFE_WAV_NO_FMT ;

					/* If this file has a SECOND fmt chunk, I don't want to know about it. */
					if (parsestage & HAVE_fmt)
						break ;

					parsestage |= HAVE_fmt ;

					psf_binheader_readf (psf, "e4", &dword) ;
					psf_log_printf (psf, "fmt  : %d\n", dword) ;

					if ((error = wav_w64_read_fmt_chunk (psf, &wav_fmt, dword)))
						return error ;

					format = wav_fmt.format ;
					break ;

			case data_MARKER :
					if ((parsestage & (HAVE_RIFF | HAVE_WAVE | HAVE_fmt)) != (HAVE_RIFF | HAVE_WAVE | HAVE_fmt))
						return SFE_WAV_NO_DATA ;

					if (psf->mode == SFM_RDWR && (parsestage & HAVE_other) != 0)
						return SFE_RDWR_BAD_HEADER ;

					parsestage |= HAVE_data ;

					psf_binheader_readf (psf, "e4", &dword) ;

					psf->datalength = dword ;
					psf->dataoffset = psf_ftell (psf) ;

					if (dword == 0 && RIFFsize == 8 && psf->filelength > 44)
					{	psf_log_printf (psf, "*** Looks like a WAV file which wasn't closed properly. Fixing it.\n") ;
						psf->datalength = dword = psf->filelength - psf->dataoffset ;
						} ;

					if (psf->datalength > psf->filelength - psf->dataoffset)
					{	psf_log_printf (psf, "data : %D (should be %D)\n", psf->datalength, psf->filelength - psf->dataoffset) ;
						psf->datalength = psf->filelength - psf->dataoffset ;
						}
					else
						psf_log_printf (psf, "data : %D\n", psf->datalength) ;

					/* Only set dataend if there really is data at the end. */
					if (psf->datalength + psf->dataoffset < psf->filelength)
						psf->dataend = psf->datalength + psf->dataoffset ;

					if (format == WAVE_FORMAT_MS_ADPCM && psf->datalength % 2)
					{	psf->datalength ++ ;
						psf_log_printf (psf, "*** Data length odd. Increasing it by 1.\n") ;
						} ;

					if (! psf->sf.seekable)
						break ;

					/* Seek past data and continue reading header. */
					psf_fseek (psf, psf->datalength, SEEK_CUR) ;

					dword = psf_ftell (psf) ;
					if (dword != (sf_count_t) (psf->dataoffset + psf->datalength))
						psf_log_printf (psf, "*** psf_fseek past end error ***\n", dword, psf->dataoffset + psf->datalength) ;
					break ;

			case fact_MARKER :
					if ((parsestage & (HAVE_RIFF | HAVE_WAVE)) != (HAVE_RIFF | HAVE_WAVE))
						return SFE_WAV_BAD_FACT ;

					parsestage |= HAVE_fact ;

					if ((parsestage & HAVE_fmt) != HAVE_fmt)
						psf_log_printf (psf, "*** Should have 'fmt ' chunk before 'fact'\n") ;

					psf_binheader_readf (psf, "e44", &dword, & (fact_chunk.frames)) ;

					if (dword > SIGNED_SIZEOF (fact_chunk))
						psf_binheader_readf (psf, "j", (int) (dword - SIGNED_SIZEOF (fact_chunk))) ;

					if (dword)
						psf_log_printf (psf, "%M : %d\n", marker, dword) ;
					else
						psf_log_printf (psf, "%M : %d (should not be zero)\n", marker, dword) ;

					psf_log_printf (psf, "  frames  : %d\n", fact_chunk.frames) ;
					break ;

			case PEAK_MARKER :
					if ((parsestage & (HAVE_RIFF | HAVE_WAVE | HAVE_fmt)) != (HAVE_RIFF | HAVE_WAVE | HAVE_fmt))
						return SFE_WAV_PEAK_B4_FMT ;

					parsestage |= HAVE_PEAK ;

					psf_binheader_readf (psf, "e4", &dword) ;

					psf_log_printf (psf, "%M : %d\n", marker, dword) ;
					if (dword != SIGNED_SIZEOF (PEAK_CHUNK) + psf->sf.channels * SIGNED_SIZEOF (PEAK_POS))
					{	psf_binheader_readf (psf, "j", dword) ;
						psf_log_printf (psf, "*** File PEAK chunk size doesn't fit with number of channels.\n") ;
						return SFE_WAV_BAD_PEAK ;
						} ;

					psf->pchunk = calloc (1, sizeof (PEAK_CHUNK) * psf->sf.channels * sizeof (PEAK_POS)) ;
					if (psf->pchunk == NULL)
						return SFE_MALLOC_FAILED ;

					/* read in rest of PEAK chunk. */
					psf_binheader_readf (psf, "e44", & (psf->pchunk->version), & (psf->pchunk->timestamp)) ;

					if (psf->pchunk->version != 1)
						psf_log_printf (psf, "  version    : %d *** (should be version 1)\n", psf->pchunk->version) ;
					else
						psf_log_printf (psf, "  version    : %d\n", psf->pchunk->version) ;

					psf_log_printf (psf, "  time stamp : %d\n", psf->pchunk->timestamp) ;
					psf_log_printf (psf, "    Ch   Position       Value\n") ;

					cptr = psf->u.cbuf ;
					for (dword = 0 ; dword < psf->sf.channels ; dword++)
					{	psf_binheader_readf (psf, "ef4", & (psf->pchunk->peaks [dword].value),
														& (psf->pchunk->peaks [dword].position)) ;

						LSF_SNPRINTF (cptr, sizeof (psf->u.cbuf), "    %2d   %-12d   %g\n",
								dword, psf->pchunk->peaks [dword].position, psf->pchunk->peaks [dword].value) ;
						cptr [sizeof (psf->u.cbuf) - 1] = 0 ;
						psf_log_printf (psf, cptr) ;
						} ;

					psf->has_peak = SF_TRUE ; /* Found PEAK chunk. */
					psf->peak_loc = ((parsestage & HAVE_data) == 0) ? SF_PEAK_START : SF_PEAK_END ;
					break ;

			case cue_MARKER :
					parsestage |= HAVE_other ;

					{	int bytesread, cue_count ;
						int id, position, chunk_id, chunk_start, block_start, offset ;

						bytesread = psf_binheader_readf (psf, "e44", &dword, &cue_count) ;
						bytesread -= 4 ; /* Remove bytes for first dword. */
						psf_log_printf (psf, "%M : %u\n  Count : %d\n", marker, dword, cue_count) ;

						while (cue_count)
						{	bytesread += psf_binheader_readf (psf, "e444444", &id, &position,
									&chunk_id, &chunk_start, &block_start, &offset) ;
							psf_log_printf (psf, "   Cue ID : %2d"
												 "  Pos : %5u  Chunk : %M"
												 "  Chk Start : %d  Blk Start : %d"
												 "  Offset : %5d\n",
									id, position, chunk_id, chunk_start, block_start, offset) ;
							cue_count -- ;
							} ;

						if (bytesread != dword)
						{	psf_log_printf (psf, "**** Chunk size weirdness (%d != %d)\n", dword, bytesread) ;
							psf_binheader_readf (psf, "j", dword - bytesread) ;
							} ;
						} ;
					break ;

			case smpl_MARKER :
					parsestage |= HAVE_other ;

					psf_binheader_readf (psf, "e4", &dword) ;
					psf_log_printf (psf, "smpl : %u\n", dword) ;

					if ((error = wav_read_smpl_chunk (psf, dword)))
						return error ;
					break ;

			case acid_MARKER :
					parsestage |= HAVE_other ;

					psf_binheader_readf (psf, "e4", &dword) ;
					psf_log_printf (psf, "acid : %u\n", dword) ;

					if ((error = wav_read_acid_chunk (psf, dword)))
						return error ;
					break ;

			case INFO_MARKER :
			case LIST_MARKER :
					parsestage |= HAVE_other ;

					if ((error = wav_subchunk_parse (psf, marker)) != 0)
						return error ;
					break ;

			case strc_MARKER : /* Multiple of 32 bytes. */

			case afsp_MARKER :
			case bext_MARKER :
			case clm_MARKER :
			case plst_MARKER :
			case DISP_MARKER :
			case MEXT_MARKER :
			case PAD_MARKER :
					parsestage |= HAVE_other ;

					psf_binheader_readf (psf, "e4", &dword) ;
					psf_log_printf (psf, "%M : %u\n", marker, dword) ;
					dword += (dword & 1) ;
					psf_binheader_readf (psf, "j", dword) ;
					break ;

			default :
					if (isprint ((marker >> 24) & 0xFF) && isprint ((marker >> 16) & 0xFF)
						&& isprint ((marker >> 8) & 0xFF) && isprint (marker & 0xFF))
					{	psf_binheader_readf (psf, "e4", &dword) ;
						psf_log_printf (psf, "*** %M : %d (unknown marker)\n", marker, dword) ;
						psf_binheader_readf (psf, "j", dword) ;
						break ;
						} ;
					if (psf_ftell (psf) & 0x03)
					{	psf_log_printf (psf, "  Unknown chunk marker at position %d. Resynching.\n", dword - 4) ;
						psf_binheader_readf (psf, "j", -3) ;
						break ;
						} ;
					psf_log_printf (psf, "*** Unknown chunk marker : %X. Exiting parser.\n", marker) ;
					done = SF_TRUE ;
					break ;
			} ;	/* switch (dword) */

		if (! psf->sf.seekable && (parsestage & HAVE_data))
			break ;

		if (psf_ftell (psf) >= psf->filelength - SIGNED_SIZEOF (dword))
		{	psf_log_printf (psf, "End\n") ;
			break ;
			} ;

		if (psf->logindex >= SIGNED_SIZEOF (psf->logbuffer) - 2)
			return SFE_LOG_OVERRUN ;
		} ; /* while (1) */

	if (! psf->dataoffset)
		return SFE_WAV_NO_DATA ;

	psf->endian = SF_ENDIAN_LITTLE ;		/* All WAV files are little endian. */

	psf_fseek (psf, psf->dataoffset, SEEK_SET) ;

	if (psf->is_pipe == 0)
	{	/*
		** Check for 'wvpk' at the start of the DATA section. Not able to
		** handle this.
		*/
		psf_binheader_readf (psf, "e4", &marker) ;
		if (marker == wvpk_MARKER || marker == OggS_MARKER)
			return SFE_WAV_WVPK_DATA ;
		} ;

	/* Seek to start of DATA section. */
	psf_fseek (psf, psf->dataoffset, SEEK_SET) ;

	psf->close = wav_close ;

	if (psf->blockwidth)
	{	if (psf->filelength - psf->dataoffset < psf->datalength)
			psf->sf.frames = (psf->filelength - psf->dataoffset) / psf->blockwidth ;
		else
			psf->sf.frames = psf->datalength / psf->blockwidth ;
		} ;

	switch (format)
	{

		case WAVE_FORMAT_EXTENSIBLE :
			/* compare GUIDs for known ones */
			if (wavex_write_guid_equal (&wav_fmt.ext.esf, &MSGUID_SUBTYPE_PCM))
				psf->sf.format = SF_FORMAT_WAVEX | u_bitwidth_to_subformat (psf->bytewidth * 8) ;
			else
			if (wavex_write_guid_equal (&wav_fmt.ext.esf, &MSGUID_SUBTYPE_MS_ADPCM))
			{	psf->sf.format = (SF_FORMAT_WAVEX | SF_FORMAT_MS_ADPCM) ;
				*blockalign = wav_fmt.msadpcm.blockalign ;
				*framesperblock = wav_fmt.msadpcm.samplesperblock ;
				}
			else if (wavex_write_guid_equal (&wav_fmt.ext.esf, &MSGUID_SUBTYPE_IEEE_FLOAT))
				psf->sf.format = SF_FORMAT_WAVEX | ((psf->bytewidth == 8) ? SF_FORMAT_DOUBLE : SF_FORMAT_FLOAT) ;
			else if (wavex_write_guid_equal (&wav_fmt.ext.esf, &MSGUID_SUBTYPE_ALAW))
				psf->sf.format = (SF_FORMAT_WAVEX | SF_FORMAT_ALAW) ;
			else if (wavex_write_guid_equal (&wav_fmt.ext.esf, &MSGUID_SUBTYPE_MULAW))
				psf->sf.format = (SF_FORMAT_WAVEX | SF_FORMAT_ULAW) ;
			else
				return SFE_UNIMPLEMENTED ;
			break ;

		case WAVE_FORMAT_PCM :
					psf->sf.format = SF_FORMAT_WAV | u_bitwidth_to_subformat (psf->bytewidth * 8) ;
					break ;

		case WAVE_FORMAT_MULAW :
		case IBM_FORMAT_MULAW :
					psf->sf.format = (SF_FORMAT_WAV | SF_FORMAT_ULAW) ;
					break ;

		case WAVE_FORMAT_ALAW :
		case IBM_FORMAT_ALAW :
					psf->sf.format = (SF_FORMAT_WAV | SF_FORMAT_ALAW) ;
					break ;

		case WAVE_FORMAT_MS_ADPCM :
					psf->sf.format = (SF_FORMAT_WAV | SF_FORMAT_MS_ADPCM) ;
					*blockalign = wav_fmt.msadpcm.blockalign ;
					*framesperblock = wav_fmt.msadpcm.samplesperblock ;
					break ;

		case WAVE_FORMAT_IMA_ADPCM :
					psf->sf.format = (SF_FORMAT_WAV | SF_FORMAT_IMA_ADPCM) ;
					*blockalign = wav_fmt.ima.blockalign ;
					*framesperblock = wav_fmt.ima.samplesperblock ;
					break ;

		case WAVE_FORMAT_GSM610 :
					psf->sf.format = (SF_FORMAT_WAV | SF_FORMAT_GSM610) ;
					break ;

		case WAVE_FORMAT_IEEE_FLOAT :
					psf->sf.format = SF_FORMAT_WAV ;
					psf->sf.format |= (psf->bytewidth == 8) ? SF_FORMAT_DOUBLE : SF_FORMAT_FLOAT ;
					break ;

		default : return SFE_UNIMPLEMENTED ;
		} ;

	return 0 ;
} /* wav_read_header */

static int
wav_write_header (SF_PRIVATE *psf, int calc_length)
{	sf_count_t	current ;
	int 		fmt_size, k, subformat, add_fact_chunk = SF_FALSE ;

	current = psf_ftell (psf) ;

	if (calc_length)
	{	psf->filelength = psf_get_filelen (psf) ;

		psf->datalength = psf->filelength - psf->dataoffset ;

		if (psf->dataend)
			psf->datalength -= psf->filelength - psf->dataend ;

		if (psf->bytewidth > 0)
			psf->sf.frames = psf->datalength / (psf->bytewidth * psf->sf.channels) ;
		} ;

	/* Reset the current header length to zero. */
	psf->header [0] = 0 ;
	psf->headindex = 0 ;
	psf_fseek (psf, 0, SEEK_SET) ;

	/* RIFF marker, length, WAVE and 'fmt ' markers. */
	if (psf->filelength < 8)
		psf_binheader_writef (psf, "etm8", RIFF_MARKER, 8) ;
	else
		psf_binheader_writef (psf, "etm8", RIFF_MARKER, psf->filelength - 8) ;

	/* WAVE and 'fmt ' markers. */
	psf_binheader_writef (psf, "emm", WAVE_MARKER, fmt_MARKER) ;

	subformat = psf->sf.format & SF_FORMAT_SUBMASK ;

	switch (subformat)
	{	case SF_FORMAT_PCM_U8 :
		case SF_FORMAT_PCM_16 :
		case SF_FORMAT_PCM_24 :
		case SF_FORMAT_PCM_32 :
					fmt_size = 2 + 2 + 4 + 4 + 2 + 2 ;

					/* fmt : format, channels, samplerate */
					psf_binheader_writef (psf, "e4224", fmt_size, WAVE_FORMAT_PCM, psf->sf.channels, psf->sf.samplerate) ;
					/*  fmt : bytespersec */
					psf_binheader_writef (psf, "e4", psf->sf.samplerate * psf->bytewidth * psf->sf.channels) ;
					/*  fmt : blockalign, bitwidth */
					psf_binheader_writef (psf, "e22", psf->bytewidth * psf->sf.channels, psf->bytewidth * 8) ;
					break ;

		case SF_FORMAT_FLOAT :
		case SF_FORMAT_DOUBLE :
					fmt_size = 2 + 2 + 4 + 4 + 2 + 2 ;

					/* fmt : format, channels, samplerate */
					psf_binheader_writef (psf, "e4224", fmt_size, WAVE_FORMAT_IEEE_FLOAT, psf->sf.channels, psf->sf.samplerate) ;
					/*  fmt : bytespersec */
					psf_binheader_writef (psf, "e4", psf->sf.samplerate * psf->bytewidth * psf->sf.channels) ;
					/*  fmt : blockalign, bitwidth */
					psf_binheader_writef (psf, "e22", psf->bytewidth * psf->sf.channels, psf->bytewidth * 8) ;

					add_fact_chunk = SF_TRUE ;
					break ;

		case SF_FORMAT_ULAW :
					fmt_size = 2 + 2 + 4 + 4 + 2 + 2 ;

					/* fmt : format, channels, samplerate */
					psf_binheader_writef (psf, "e4224", fmt_size, WAVE_FORMAT_MULAW, psf->sf.channels, psf->sf.samplerate) ;
					/*  fmt : bytespersec */
					psf_binheader_writef (psf, "e4", psf->sf.samplerate * psf->bytewidth * psf->sf.channels) ;
					/*  fmt : blockalign, bitwidth */
					psf_binheader_writef (psf, "e22", psf->bytewidth * psf->sf.channels, 8) ;

					add_fact_chunk = SF_TRUE ;
					break ;

		case SF_FORMAT_ALAW :
					fmt_size = 2 + 2 + 4 + 4 + 2 + 2 ;

					/* fmt : format, channels, samplerate */
					psf_binheader_writef (psf, "e4224", fmt_size, WAVE_FORMAT_ALAW, psf->sf.channels, psf->sf.samplerate) ;
					/*  fmt : bytespersec */
					psf_binheader_writef (psf, "e4", psf->sf.samplerate * psf->bytewidth * psf->sf.channels) ;
					/*  fmt : blockalign, bitwidth */
					psf_binheader_writef (psf, "e22", psf->bytewidth * psf->sf.channels, 8) ;

					add_fact_chunk = SF_TRUE ;
					break ;

		/* Lite remove start */
		case SF_FORMAT_IMA_ADPCM :
					{	int blockalign, framesperblock, bytespersec ;

						blockalign		= wav_w64_srate2blocksize (psf->sf.samplerate * psf->sf.channels) ;
						framesperblock	= 2 * (blockalign - 4 * psf->sf.channels) / psf->sf.channels + 1 ;
						bytespersec		= (psf->sf.samplerate * blockalign) / framesperblock ;

						/* fmt chunk. */
						fmt_size = 2 + 2 + 4 + 4 + 2 + 2 + 2 + 2 ;

						/* fmt : size, WAV format type, channels, samplerate, bytespersec */
						psf_binheader_writef (psf, "e42244", fmt_size, WAVE_FORMAT_IMA_ADPCM,
									psf->sf.channels, psf->sf.samplerate, bytespersec) ;

						/* fmt : blockalign, bitwidth, extrabytes, framesperblock. */
						psf_binheader_writef (psf, "e2222", blockalign, 4, 2, framesperblock) ;
						} ;

					add_fact_chunk = SF_TRUE ;
					break ;

		case SF_FORMAT_MS_ADPCM :
					{	int	blockalign, framesperblock, bytespersec, extrabytes ;

						blockalign		= wav_w64_srate2blocksize (psf->sf.samplerate * psf->sf.channels) ;
						framesperblock	= 2 + 2 * (blockalign - 7 * psf->sf.channels) / psf->sf.channels ;
						bytespersec		= (psf->sf.samplerate * blockalign) / framesperblock ;

						/* fmt chunk. */
						extrabytes	= 2 + 2 + MSADPCM_ADAPT_COEFF_COUNT * (2 + 2) ;
						fmt_size	= 2 + 2 + 4 + 4 + 2 + 2 + 2 + extrabytes ;

						/* fmt : size, WAV format type, channels. */
						psf_binheader_writef (psf, "e422", fmt_size, WAVE_FORMAT_MS_ADPCM, psf->sf.channels) ;

						/* fmt : samplerate, bytespersec. */
						psf_binheader_writef (psf, "e44", psf->sf.samplerate, bytespersec) ;

						/* fmt : blockalign, bitwidth, extrabytes, framesperblock. */
						psf_binheader_writef (psf, "e22222", blockalign, 4, extrabytes, framesperblock, 7) ;

						msadpcm_write_adapt_coeffs (psf) ;
						} ;

					add_fact_chunk = SF_TRUE ;
					break ;
		/* Lite remove end */

		case SF_FORMAT_GSM610 :
					{	int	blockalign, framesperblock, bytespersec ;

						blockalign		= WAV_W64_GSM610_BLOCKSIZE ;
						framesperblock	= WAV_W64_GSM610_SAMPLES ;
						bytespersec		= (psf->sf.samplerate * blockalign) / framesperblock ;

						/* fmt chunk. */
						fmt_size = 2 + 2 + 4 + 4 + 2 + 2 + 2 + 2 ;

						/* fmt : size, WAV format type, channels. */
						psf_binheader_writef (psf, "e422", fmt_size, WAVE_FORMAT_GSM610, psf->sf.channels) ;

						/* fmt : samplerate, bytespersec. */
						psf_binheader_writef (psf, "e44", psf->sf.samplerate, bytespersec) ;

						/* fmt : blockalign, bitwidth, extrabytes, framesperblock. */
						psf_binheader_writef (psf, "e2222", blockalign, 0, 2, framesperblock) ;
						} ;

					add_fact_chunk = SF_TRUE ;
					break ;

		default : 	return SFE_UNIMPLEMENTED ;
		} ;

	if (add_fact_chunk)
		psf_binheader_writef (psf, "etm48", fact_MARKER, 4, psf->sf.frames) ;

	if (psf->str_flags & SF_STR_LOCATE_START)
		wav_write_strings (psf, SF_STR_LOCATE_START) ;

	if (psf->has_peak && psf->peak_loc == SF_PEAK_START)
	{	psf_binheader_writef (psf, "em4", PEAK_MARKER,
			sizeof (PEAK_CHUNK) + psf->sf.channels * sizeof (PEAK_POS)) ;
		psf_binheader_writef (psf, "e44", 1, time (NULL)) ;
		for (k = 0 ; k < psf->sf.channels ; k++)
			psf_binheader_writef (psf, "ef4", psf->pchunk->peaks [k].value, psf->pchunk->peaks [k].position) ;
		} ;

	psf_binheader_writef (psf, "etm8", data_MARKER, psf->datalength) ;
	psf_fwrite (psf->header, psf->headindex, 1, psf) ;
	if (psf->error)
		return psf->error ;

	psf->dataoffset = psf->headindex ;

	if (current < psf->dataoffset)
		psf_fseek (psf, psf->dataoffset, SEEK_SET) ;
	else if (current > 0)
		psf_fseek (psf, current, SEEK_SET) ;

	return psf->error ;
} /* wav_write_header */



static int
wavex_write_guid_equal (const EXT_SUBFORMAT * first, const EXT_SUBFORMAT * second)
{	return !memcmp (first, second, sizeof (EXT_SUBFORMAT)) ;
} /* wavex_write_guid_equal */


static void
wavex_write_guid (SF_PRIVATE *psf, const EXT_SUBFORMAT * subformat)
{
	psf_binheader_writef (psf, "e422b", subformat->esf_field1,
					subformat->esf_field2, subformat->esf_field3,
					subformat->esf_field4, 8) ;
} /* wavex_write_guid */


static int
wavex_write_header (SF_PRIVATE *psf, int calc_length)
{	sf_count_t	current ;
	int 		fmt_size, k, subformat, add_fact_chunk = SF_FALSE ;

	current = psf_ftell (psf) ;

	if (calc_length)
	{	psf->filelength = psf_get_filelen (psf) ;

		psf->datalength = psf->filelength - psf->dataoffset ;

		if (psf->dataend)
			psf->datalength -= psf->filelength - psf->dataend ;

		if (psf->bytewidth > 0)
			psf->sf.frames = psf->datalength / (psf->bytewidth * psf->sf.channels) ;
		} ;


	/* Reset the current header length to zero. */
	psf->header [0] = 0 ;
	psf->headindex = 0 ;
	psf_fseek (psf, 0, SEEK_SET) ;

	/* RIFF marker, length, WAVE and 'fmt ' markers. */
	if (psf->filelength < 8)
		psf_binheader_writef (psf, "etm8", RIFF_MARKER, 8) ;
	else
		psf_binheader_writef (psf, "etm8", RIFF_MARKER, psf->filelength - 8) ;

	/* WAVE and 'fmt ' markers. */
	psf_binheader_writef (psf, "emm", WAVE_MARKER, fmt_MARKER) ;

	subformat = psf->sf.format & SF_FORMAT_SUBMASK ;

	/* initial section (same for all, it appears) */
	switch (subformat)
	{	case SF_FORMAT_PCM_U8 :
		case SF_FORMAT_PCM_16 :
		case SF_FORMAT_PCM_24 :
		case SF_FORMAT_PCM_32 :
		case SF_FORMAT_FLOAT :
		case SF_FORMAT_DOUBLE :
		case SF_FORMAT_ULAW :
		case SF_FORMAT_ALAW :
			fmt_size = 2 + 2 + 4 + 4 + 2 + 2 + 2 + 2 + 4 + 4 + 2 + 2 + 8 ;

			/* fmt : format, channels, samplerate */
			psf_binheader_writef (psf, "e4224", fmt_size, WAVE_FORMAT_EXTENSIBLE, psf->sf.channels, psf->sf.samplerate) ;
			/*  fmt : bytespersec */
			psf_binheader_writef (psf, "e4", psf->sf.samplerate * psf->bytewidth * psf->sf.channels) ;
			/*  fmt : blockalign, bitwidth */
			psf_binheader_writef (psf, "e22", psf->bytewidth * psf->sf.channels, psf->bytewidth * 8) ;

			/* cbSize 22 is sizeof (WAVEFORMATEXTENSIBLE) - sizeof (WAVEFORMATEX) */
			psf_binheader_writef (psf, "e2", 22) ;

			/* wValidBitsPerSample, for our use same as bitwidth as we use it fully */
			psf_binheader_writef (psf, "e2", psf->bytewidth * 8) ;

			if (psf->sf.channels == 2)
				psf_binheader_writef (psf, "e4", 0x1 | 0x2 ) ;	/* dwChannelMask front left and right */
			else
				psf_binheader_writef (psf, "e4", 0) ;			/* dwChannelMask = 0 when in doubt */
			break ;

		case SF_FORMAT_MS_ADPCM : /* todo, GUID exists might have different header as per wav_write_header */
		default : return SFE_UNIMPLEMENTED ;
		} ;

	/* GUI section, different for each */

	switch (subformat)
	{	case SF_FORMAT_PCM_U8 :
		case SF_FORMAT_PCM_16 :
		case SF_FORMAT_PCM_24 :
		case SF_FORMAT_PCM_32 :
			wavex_write_guid (psf, &MSGUID_SUBTYPE_PCM) ;
			break ;

		case SF_FORMAT_FLOAT :
		case SF_FORMAT_DOUBLE :
			wavex_write_guid (psf, &MSGUID_SUBTYPE_IEEE_FLOAT) ;
			add_fact_chunk = SF_TRUE ;
			break ;

		case SF_FORMAT_ULAW :
			wavex_write_guid (psf, &MSGUID_SUBTYPE_MULAW) ;
			add_fact_chunk = SF_TRUE ;
			break ;

		case SF_FORMAT_ALAW :
			wavex_write_guid (psf, &MSGUID_SUBTYPE_ALAW) ;
			add_fact_chunk = SF_TRUE ;
			break ;

		case SF_FORMAT_MS_ADPCM : /* todo, GUID exists */

		default : return SFE_UNIMPLEMENTED ;
		} ;


	if (add_fact_chunk)
		psf_binheader_writef (psf, "etm48", fact_MARKER, 4, psf->sf.frames) ;

	if (psf->str_flags & SF_STR_LOCATE_START)
		wav_write_strings (psf, SF_STR_LOCATE_START) ;

	if (psf->has_peak && psf->peak_loc == SF_PEAK_START)
	{	psf_binheader_writef (psf, "em4", PEAK_MARKER,
			sizeof (PEAK_CHUNK) + psf->sf.channels * sizeof (PEAK_POS)) ;
		psf_binheader_writef (psf, "e44", 1, time (NULL)) ;
		for (k = 0 ; k < psf->sf.channels ; k++)
			psf_binheader_writef (psf, "ef4", psf->pchunk->peaks [k].value, psf->pchunk->peaks [k].position) ;
		} ;

	psf_binheader_writef (psf, "etm8", data_MARKER, psf->datalength) ;
	psf_fwrite (psf->header, psf->headindex, 1, psf) ;
	if (psf->error)
		return psf->error ;

	psf->dataoffset = psf->headindex ;

	if (current < psf->dataoffset)
		psf_fseek (psf, psf->dataoffset, SEEK_SET) ;
	else if (current > 0)
		psf_fseek (psf, current, SEEK_SET) ;

	return psf->error ;
} /* wavex_write_header */



static int
wav_write_tailer (SF_PRIVATE *psf)
{	int		k ;

	/* Reset the current header buffer length to zero. */
	psf->header [0] = 0 ;
	psf->headindex = 0 ;

	psf->dataend = psf_fseek (psf, 0, SEEK_END) ;

	/* Add a PEAK chunk if requested. */
	if (psf->has_peak && psf->peak_loc == SF_PEAK_END)
	{	psf_binheader_writef (psf, "em4", PEAK_MARKER,
			sizeof (PEAK_CHUNK) + psf->sf.channels * sizeof (PEAK_POS)) ;
		psf_binheader_writef (psf, "e44", 1, time (NULL)) ;
		for (k = 0 ; k < psf->sf.channels ; k++)
			psf_binheader_writef (psf, "ef4", psf->pchunk->peaks [k].value, psf->pchunk->peaks [k].position) ;
		} ;

	if (psf->str_flags & SF_STR_LOCATE_END)
		wav_write_strings (psf, SF_STR_LOCATE_END) ;

	/* Write the tailer. */
	if (psf->headindex > 0)
		psf_fwrite (psf->header, psf->headindex, 1, psf) ;

	return 0 ;
} /* wav_write_tailer */

static void
wav_write_strings (SF_PRIVATE *psf, int location)
{	int	k, prev_head_index, saved_head_index ;

	prev_head_index = psf->headindex + 4 ;

	psf_binheader_writef (psf, "em4m", LIST_MARKER, 0xBADBAD, INFO_MARKER) ;

	for (k = 0 ; k < SF_MAX_STRINGS ; k++)
	{	if (psf->strings [k].type == 0)
			break ;
		if (psf->strings [k].flags != location)
			continue ;

		switch (psf->strings [k].type)
		{	case SF_STR_SOFTWARE :
				psf_binheader_writef (psf, "ems", ISFT_MARKER, psf->strings [k].str) ;
				break ;

			case SF_STR_TITLE :
				psf_binheader_writef (psf, "ems", INAM_MARKER, psf->strings [k].str) ;
				break ;

			case SF_STR_COPYRIGHT :
				psf_binheader_writef (psf, "ems", ICOP_MARKER, psf->strings [k].str) ;
				break ;

			case SF_STR_ARTIST :
				psf_binheader_writef (psf, "ems", IART_MARKER, psf->strings [k].str) ;
				break ;

			case SF_STR_COMMENT :
				psf_binheader_writef (psf, "ems", ICMT_MARKER, psf->strings [k].str) ;
				break ;

			case SF_STR_DATE :
				psf_binheader_writef (psf, "ems", ICRD_MARKER, psf->strings [k].str) ;
				break ;
			} ;
		} ;

	saved_head_index = psf->headindex ;
	psf->headindex = prev_head_index ;
	psf_binheader_writef (psf, "e4", saved_head_index - prev_head_index - 4) ;
	psf->headindex = saved_head_index ;

} /* wav_write_strings */

static int
wav_close (SF_PRIVATE *psf)
{
	if (psf->mode == SFM_WRITE || psf->mode == SFM_RDWR)
	{	wav_write_tailer (psf) ;

		if ((psf->sf.format & SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV)
			wav_write_header (psf, SF_TRUE) ;
		else
			wavex_write_header (psf, SF_TRUE) ;
		} ;

	return 0 ;
} /* wav_close */

static int
wav_command (SF_PRIVATE *psf, int command, void *data, int datasize)
{
	/* Avoid compiler warnings. */
	psf = psf ;
	data = data ;
	datasize = datasize ;

	switch (command)
	{	default : break ;
		} ;

	return 0 ;
} /* wav_command */

static int
wav_subchunk_parse (SF_PRIVATE *psf, int chunk)
{	sf_count_t	current_pos ;
	char		*cptr ;
	int 		dword, bytesread, length ;

	current_pos = psf_fseek (psf, 0, SEEK_CUR) ;

	bytesread = psf_binheader_readf (psf, "e4", &length) ;

	if (current_pos + length > psf->filelength)
	{	psf_log_printf (psf, "%M : %d (should be %d)\n", chunk, length, (int) (psf->filelength - current_pos)) ;
		length = psf->filelength - current_pos ;
		}
	else
		psf_log_printf (psf, "%M : %d\n", chunk, length) ;


	while (bytesread < length)
	{	bytesread += psf_binheader_readf (psf, "m", &chunk) ;

		switch (chunk)
		{	case adtl_MARKER :
			case INFO_MARKER :
					/* These markers don't contain anything. */
					psf_log_printf (psf, "  %M\n", chunk) ;
					break ;

			case data_MARKER:
					psf_log_printf (psf, "  %M inside a LIST block??? Backing out.\n", chunk) ;
					/* Jump back four bytes and return to caller. */
					psf_binheader_readf (psf, "j", -4) ;
					return 0 ;

			case ISFT_MARKER :
			case ICOP_MARKER :
			case IARL_MARKER :
			case IART_MARKER :
			case ICMT_MARKER :
			case ICRD_MARKER :
			case IENG_MARKER :

			case INAM_MARKER :
			case IPRD_MARKER :
			case ISBJ_MARKER :
			case ISRC_MARKER :
					bytesread += psf_binheader_readf (psf, "e4", &dword) ;
					dword += (dword & 1) ;
					if (dword > SIGNED_SIZEOF (psf->u.cbuf))
					{	psf_log_printf (psf, "  *** %M : %d (too big)\n", chunk, dword) ;
						return SFE_INTERNAL ;
						} ;

					cptr = psf->u.cbuf ;
					psf_binheader_readf (psf, "b", cptr, dword) ;
					bytesread += dword ;
					cptr [dword - 1] = 0 ;
					psf_log_printf (psf, "    %M : %s\n", chunk, cptr) ;
					break ;

			case labl_MARKER :
					{	int mark_id ;

						bytesread += psf_binheader_readf (psf, "e44", &dword, &mark_id) ;
						dword -= 4 ;
						dword += (dword & 1) ;
						if (dword > SIGNED_SIZEOF (psf->u.cbuf))
						{	psf_log_printf (psf, "  *** %M : %d (too big)\n", chunk, dword) ;
							return SFE_INTERNAL ;
							} ;

						cptr = psf->u.cbuf ;
						psf_binheader_readf (psf, "b", cptr, dword) ;
						bytesread += dword ;
						cptr [dword - 1] = 0 ;
						psf_log_printf (psf, "    %M : %d : %s\n", chunk, mark_id, cptr) ;
						} ;
					break ;


			case DISP_MARKER :
			case ltxt_MARKER :
			case note_MARKER :
					bytesread += psf_binheader_readf (psf, "e4", &dword) ;
					dword += (dword & 1) ;
					psf_binheader_readf (psf, "j", dword) ;
					bytesread += dword ;
					psf_log_printf (psf, "    %M : %d\n", chunk, dword) ;
					break ;

			default :
					psf_binheader_readf (psf, "e4", &dword) ;
					bytesread += sizeof (dword) ;
					dword += (dword & 1) ;
					psf_binheader_readf (psf, "j", dword) ;
					bytesread += dword ;
					psf_log_printf (psf, "    *** %M : %d\n", chunk, dword) ;
					if (dword > length)
						return 0 ;
					break ;
			} ;

		switch (chunk)
		{	case ISFT_MARKER :
					psf_store_string (psf, SF_STR_SOFTWARE, psf->u.cbuf) ;
					break ;
			case ICOP_MARKER :
					psf_store_string (psf, SF_STR_COPYRIGHT, psf->u.cbuf) ;
					break ;
			case INAM_MARKER :
					psf_store_string (psf, SF_STR_TITLE, psf->u.cbuf) ;
					break ;
			case IART_MARKER :
					psf_store_string (psf, SF_STR_ARTIST, psf->u.cbuf) ;
					break ;
			case ICMT_MARKER :
					psf_store_string (psf, SF_STR_COMMENT, psf->u.cbuf) ;
					break ;
			case ICRD_MARKER :
					psf_store_string (psf, SF_STR_DATE, psf->u.cbuf) ;
					break ;
			} ;

		if (psf->logindex >= SIGNED_SIZEOF (psf->logbuffer) - 2)
			return SFE_LOG_OVERRUN ;
		} ;

	current_pos = psf_fseek (psf, 0, SEEK_CUR) - current_pos ;

	if (current_pos - 4 != length)
		psf_log_printf (psf, "**** Bad chunk length %d sbould be %D\n", length, current_pos - 4) ;

	return 0 ;
} /* wav_subchunk_parse */

static int
wav_read_smpl_chunk (SF_PRIVATE *psf, unsigned int chunklen)
{	unsigned int bytesread = 0, dword, sampler_data, loop_count ;
	int k ;

	chunklen += (chunklen & 1) ;

	bytesread += psf_binheader_readf (psf, "e4", &dword) ;
	psf_log_printf (psf, "  Manufacturer : %X\n", dword) ;

	bytesread += psf_binheader_readf (psf, "e4", &dword) ;
	psf_log_printf (psf, "  Product      : %u\n", dword) ;

	bytesread += psf_binheader_readf (psf, "e4", &dword) ;
	psf_log_printf (psf, "  Period       : %u nsec\n", dword) ;

	bytesread += psf_binheader_readf (psf, "e4", &dword) ;
	psf_log_printf (psf, "  Midi Note    : %u\n", dword) ;

	bytesread += psf_binheader_readf (psf, "e4", &dword) ;
	if (dword != 0)
	{	LSF_SNPRINTF (psf->u.cbuf, sizeof (psf->u.cbuf), "%f",
				 (1.0 * 0x80000000) / ((unsigned int) dword)) ;
		psf_log_printf (psf, "  Pitch Fract. : %s\n", psf->u.cbuf) ;
		}
	else
		psf_log_printf (psf, "  Pitch Fract. : 0\n") ;

	bytesread += psf_binheader_readf (psf, "e4", &dword) ;
	psf_log_printf (psf, "  SMPTE Format : %u\n", dword) ;

	bytesread += psf_binheader_readf (psf, "e4", &dword) ;
	LSF_SNPRINTF (psf->u.cbuf, sizeof (psf->u.cbuf), "%02d:%02d:%02d %02d",
		 (dword >> 24) & 0x7F, (dword >> 16) & 0x7F, (dword >> 8) & 0x7F, dword & 0x7F) ;
	psf_log_printf (psf, "  SMPTE Offset : %s\n", psf->u.cbuf) ;

	bytesread += psf_binheader_readf (psf, "e4", &loop_count) ;
	psf_log_printf (psf, "  Loop Count   : %u\n", loop_count) ;

	/* Sampler Data holds the number of data bytes after the CUE chunks which
	** is not actually CUE data. Display value after CUE data.
	*/
	bytesread += psf_binheader_readf (psf, "e4", &sampler_data) ;

	while (loop_count > 0 && chunklen - bytesread >= 24)
	{
		bytesread += psf_binheader_readf (psf, "e4", &dword) ;
		psf_log_printf (psf, "    Cue ID : %2u", dword) ;

		bytesread += psf_binheader_readf (psf, "e4", &dword) ;
		psf_log_printf (psf, "  Type : %2u", dword) ;

		bytesread += psf_binheader_readf (psf, "e4", &dword) ;
		psf_log_printf (psf, "  Start : %5u", dword) ;

		bytesread += psf_binheader_readf (psf, "e4", &dword) ;
		psf_log_printf (psf, "  End : %5u", dword) ;

		bytesread += psf_binheader_readf (psf, "e4", &dword) ;
		psf_log_printf (psf, "  Fraction : %5u", dword) ;

		bytesread += psf_binheader_readf (psf, "e4", &dword) ;
		psf_log_printf (psf, "  Count : %5u\n", dword) ;

		loop_count -- ;
		} ;

	if (chunklen - bytesread == 0)
	{	if (sampler_data != 0)
			psf_log_printf (psf, "  Sampler Data : %u (should be 0)\n", sampler_data) ;
		else
			psf_log_printf (psf, "  Sampler Data : %u\n", sampler_data) ;
		}
	else
	{	if (sampler_data != chunklen - bytesread)
		{	psf_log_printf (psf, "  Sampler Data : %u (should have been %u)\n", sampler_data, chunklen - bytesread) ;
			sampler_data = chunklen - bytesread ;
			}
		else
			psf_log_printf (psf, "  Sampler Data : %u\n", sampler_data) ;

		psf_log_printf (psf, "      ") ;
		for (k = 0 ; k < (int) sampler_data ; k++)
		{	char ch ;

			if (k > 0 && (k % 20) == 0)
				psf_log_printf (psf, "\n      ") ;

			bytesread += psf_binheader_readf (psf, "1", &ch) ;
			psf_log_printf (psf, "%02X ", ch & 0xFF) ;
			} ;

		psf_log_printf (psf, "\n") ;
		} ;

	return 0 ;
} /* wav_read_smpl_chunk */

/*
** The acid chunk goes a little something like this:
**
** 4 bytes          'acid'
** 4 bytes (int)     length of chunk starting at next byte
**
** 4 bytes (int)     type of file:
**        this appears to be a bit mask,however some combinations
**        are probably impossible and/or qualified as "errors"
**
**        0x01 On: One Shot         Off: Loop
**        0x02 On: Root note is Set Off: No root
**        0x04 On: Stretch is On,   Off: Strech is OFF
**        0x08 On: Disk Based       Off: Ram based
**        0x10 On: ??????????       Off: ????????? (Acidizer puts that ON)
**
** 2 bytes (short)      root note
**        if type 0x10 is OFF : [C,C#,(...),B] -> [0x30 to 0x3B]
**        if type 0x10 is ON  : [C,C#,(...),B] -> [0x3C to 0x47]
**         (both types fit on same MIDI pitch albeit different octaves, so who cares)
**
** 2 bytes (short)      ??? always set to 0x8000
** 4 bytes (float)      ??? seems to be always 0
** 4 bytes (int)        number of beats
** 2 bytes (short)      meter denominator   //always 4 in SF/ACID
** 2 bytes (short)      meter numerator     //always 4 in SF/ACID
**                      //are we sure about the order?? usually its num/denom
** 4 bytes (float)      tempo
**
*/

static int
wav_read_acid_chunk (SF_PRIVATE *psf, unsigned int chunklen)
{	unsigned int bytesread = 0 ;
	int	beats, flags ;
	short rootnote, q1, meter_denom, meter_numer ;
	float q2, tempo ;

	chunklen += (chunklen & 1) ;

	bytesread += psf_binheader_readf (psf, "e422f", &flags, &rootnote, &q1, &q2) ;

	LSF_SNPRINTF (psf->u.cbuf, sizeof (psf->u.cbuf), "%f", q2) ;

	psf_log_printf (psf, "  Flags     : 0x%04x (%s,%s,%s,%s,%s)\n", flags,
			(flags & 0x01) ? "OneShot" : "Loop",
			(flags & 0x02) ? "RootNoteValid" : "RootNoteInvalid",
			(flags & 0x04) ? "StretchOn" : "StretchOff",
			(flags & 0x08) ? "DiskBased" : "RAMBased",
			(flags & 0x10) ? "??On" : "??Off") ;

	psf_log_printf (psf, "  Root note : 0x%x\n  ????      : 0x%04x\n  ????      : %s\n",
				rootnote, q1, psf->u.cbuf) ;

	bytesread += psf_binheader_readf (psf, "e422f", &beats, &meter_denom, &meter_numer, &tempo) ;
	LSF_SNPRINTF (psf->u.cbuf, sizeof (psf->u.cbuf), "%f", tempo) ;
	psf_log_printf (psf, "  Beats     : %d\n  Meter     : %d/%d\n  Tempo     : %s\n",
				beats, meter_numer, meter_denom, psf->u.cbuf) ;

	psf_binheader_readf (psf, "j", chunklen - bytesread) ;

	if ((psf->loop_info = calloc (1, sizeof (SF_LOOP_INFO))) == NULL)
		return SFE_MALLOC_FAILED ;

	psf->loop_info->time_sig_num	= meter_numer ;
	psf->loop_info->time_sig_den	= meter_denom ;
	psf->loop_info->loop_mode		= (flags & 0x01) ? SF_LOOP_NONE : SF_LOOP_FORWARD ;
	psf->loop_info->num_beats		= beats ;
	psf->loop_info->bpm				= tempo ;
	psf->loop_info->root_key		= (flags & 0x02) ? rootnote : -1 ;

	return 0 ;
} /* wav_read_acid_chunk */

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: 9c551689-a1d8-4905-9f56-26a204374f18
*/
