/*
** Copyright (C) 2002-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#include "sndfile.h"
#include "config.h"
#include "sfendian.h"
#include "common.h"

#if (ENABLE_EXPERIMENTAL_CODE == 0)

int
ogg_open	(SF_PRIVATE *psf)
{	if (psf)
		return SFE_UNIMPLEMENTED ;
	return (psf && 0) ;
} /* ogg_open */

#else

#define	SFE_OGG_NOT_OGG	666

/*------------------------------------------------------------------------------
** Macros to handle big/little endian issues.
*/

#define ALAW_MARKER		MAKE_MARKER ('A', 'L', 'a', 'w')
#define SOUN_MARKER		MAKE_MARKER ('S', 'o', 'u', 'n')
#define DFIL_MARKER		MAKE_MARKER ('d', 'F', 'i', 'l')

/*------------------------------------------------------------------------------
** Private static functions.
*/

static int	ogg_read_header (SF_PRIVATE *psf) ;

/*------------------------------------------------------------------------------
** Public function.
*/

int
ogg_open (SF_PRIVATE *psf)
{	OGG_PRIVATE *pogg ;
	int	subformat, error = 0 ;

	if (psf->mode == SFM_RDWR)
		return SFE_UNIMPLEMENTED ;

	psf->sf.sections = 1 ;

	psf->datalength = psf->filelength ;
	psf->dataoffset = 0 ;
	psf->blockwidth = 0 ;
	psf->bytewidth = 1 ;

	if (! (pogg = calloc (1, sizeof (OGG_PRIVATE))))
		return SFE_MALLOC_FAILED ;
	psf->fdata = pogg ;

	if (psf->mode == SFM_READ)
	{	if ((error = pogg_read_header (psf)))
			return error ;
		} ;

	if (psf->mode == SFM_WRITE)
	{	psf->str_flags = SF_STR_ALLOW_START ;

		if ((error = pogg_write_header (psf)))
			return error ;
		} ;

	if ((psf->sf.format & SF_FORMAT_TYPEMASK) == 0)
		return	SFE_BAD_OPEN_FORMAT ;

	subformat = psf->sf.format & SF_FORMAT_SUBMASK ;
	if (subformat == 0)
		return SFE_BAD_OPEN_FORMAT ;

	return pogg_init (psf) ;
} /* ogg_open */

/*------------------------------------------------------------------------------
** Private functions
*/

static int
pogg_init (SF_PRIVATE * psf)
{
	psf->close = pogg_close ;

	if (psf->mode == SFM_READ)
	{	/* set the virtual functions for reading */
		psf->read_short = pogg_read_s ;
		psf->read_int = pogg_read_i ;
		psf->read_float = pogg_read_f ;
		psf->read_double = pogg_read_d ;

		/* set the virtual function for seeking */
		psf->seek = pogg_seek ;
		} ;

	if (psf->mode == SFM_WRITE)
	{	/* set the virtual functions for writing */
		psf->write_short = pogg_write_s ;
		psf->write_int = pogg_write_i ;
		psf->write_float = pogg_write_f ;
		psf->write_double = pogg_write_d ;
		} ;

	return 0 ;
} /* pogg_init */

static int
pogg_close (SF_PRIVATE * psf)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	long n ;

	if (psf->mode == SFM_READ)
	{	if (pogg->cache_pcm != NULL)
			free (pogg->cache_pcm) ;
		/* MUST NOT free pogg->ptr, it is a pointer into the user's buffers */
		} ;

	if (psf->mode == SFM_WRITE)
	{	fish_sound_flush (pogg->fsound) ;
		while ((n = oggz_write (pogg->oggz, 1024)) > 0) ;
		} ;

	if (pogg->oggz)
			oggz_close (pogg->oggz) ;
	if (pogg->fsound)
		fish_sound_delete (pogg->fsound) ;

	return 0 ;
} /* pogg_close */


/*------------------------------------------------------------------------------
** OggzIO methods
*/

static size_t
pogg_io_read (void * user_handle, void * buf, size_t n)
{	SF_PRIVATE * psf = (SF_PRIVATE *) user_handle ;

	return (size_t) psf_fread (buf, 1, n, psf) ;
} /* pogg_io_read */

static int
pogg_io_seek (void * user_handle, long offset, int whence)
{	SF_PRIVATE * psf = (SF_PRIVATE *) user_handle ;

	return (size_t) psf_fseek (psf, offset, whence) ;
} /* pogg_io_seek */

static long
pogg_io_tell (void * user_handle)
{	SF_PRIVATE * psf = (SF_PRIVATE *) user_handle ;

	return (size_t) psf_ftell (psf) ;
} /* pogg_io_tell */

static size_t
pogg_io_write (void * user_handle, void * buf, size_t n)
{	SF_PRIVATE * psf = (SF_PRIVATE *) user_handle ;

	return (size_t) psf_fwrite (buf, 1, n, psf) ;
} /* pogg_io_write */

/*------------------------------------------------------------------------------
** Read last packet -- set the number of frames to be the last recorded
** granulepos.
*/

static int
pogg_read_last_packet (OGGZ * oggz, ogg_packet * op, long serialno, void * data)
{	SF_PRIVATE * psf = (SF_PRIVATE *) data ;

	/* Avoid compiler warning. */
	oggz = NULL ;
	serialno = 0 ;

	if (op->granulepos == -1)
		return OGGZ_CONTINUE ;

	psf->sf.frames = op->granulepos ;

	return OGGZ_STOP_OK ;
} /* pogg_read_least_packet */

/*------------------------------------------------------------------------------
** Decode header -- by the time FishSound calls this, all header codebooks etc.
** have been parsed and the Oggz is ready for seeking.
*/

static int
pogg_decode_header (FishSound * fsound, float ** pcm, long frames, void * user_data)
{	SF_PRIVATE * psf = (SF_PRIVATE *) user_data ;
	FishSoundInfo fsinfo ;
	const FishSoundComment * comment ;

	/* Avoid compiler warnings. */
	pcm = NULL ;
	frames = 0 ;

	fish_sound_command (fsound, FISH_SOUND_GET_INFO, &fsinfo, sizeof (FishSoundInfo)) ;

	switch (fsinfo.format)
	{	case FISH_SOUND_VORBIS :
			psf_log_printf (psf, "Vorbis\n") ;
			psf->sf.format |= SF_FORMAT_VORBIS ;
			break ;
		case FISH_SOUND_SPEEX :
			psf_log_printf (psf, "Speex\n") ;
			psf->sf.format |= SF_FORMAT_SPEEX ;
			break ;
		default :
			psf_log_printf (psf, "Unknown Ogg codec\n") ;
			break ;
		} ;

	psf->sf.samplerate = fsinfo.samplerate ;
	psf->sf.channels = fsinfo.channels ;

	/* Get comments */
	for (comment = fish_sound_comment_first (fsound) ; comment ;
						comment = fish_sound_comment_next (fsound, comment))
	{	psf_log_printf (psf, "%s : %s\n", comment->name, comment->value) ;

		if (strcasecmp (comment->name, "TITLE") == 0)
			psf_store_string (psf, SF_STR_TITLE, comment->value) ;
		else if (strcasecmp (comment->name, "COPYRIGHT") == 0)
			psf_store_string (psf, SF_STR_COPYRIGHT, comment->value) ;
		else if (strcasecmp (comment->name, "ENCODER") == 0)
			psf_store_string (psf, SF_STR_SOFTWARE, comment->value) ;
		else if (strcasecmp (comment->name, "ARTIST") == 0)
			psf_store_string (psf, SF_STR_ARTIST, comment->value) ;
		else if (strcasecmp (comment->name, "DATE") == 0)
			psf_store_string (psf, SF_STR_DATE, comment->value) ;
		else if (strcasecmp (comment->name, "author") == 0)
		{	/* speexenc provides this */
			psf_store_string (psf, SF_STR_ARTIST, comment->value) ;
			} ;
		} ;

	puts (psf->logbuffer) ;

	return 1 ;
} /* pogg_decode_header */

static int
pogg_read_header_packet (OGGZ * oggz, ogg_packet * op, long serialno, void * data)
{	SF_PRIVATE * psf = (SF_PRIVATE *) data ;
	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	int format ;

	/* Avoid compiler warning. */
	oggz = NULL ;

	if (pogg->serialno == -1)
		psf_log_printf (psf, "Read Ogg packet header : [%s]\n", op->packet) ;

	if (pogg->serialno == -1 && op->bytes >= 8)
	{	format = fish_sound_identify (op->packet, 8) ;
		if (format == FISH_SOUND_VORBIS || format == FISH_SOUND_SPEEX)
		{	/*
			** Detect this is (probably) the audio stream. Don't set the subformat
			** yet, do that in the decoded callback, once FishSound has had a proper
			** look at all the headers and codebooks etc. and the file is ready for
			** decoding and seeking. We use the value of (psf->sf.format & _SUBMASK)
			** below to determine whether the headers have all been read or not.
			*/
			pogg->serialno = serialno ;
			}
		else if (strncmp (op->packet, "Annodex", 8) == 0)
		{	/* The overall stream encpasulation is Annodex */
			psf->sf.format = SF_FORMAT_ANX ;
			} ;
		} ;

	if (serialno == pogg->serialno)
		fish_sound_decode (pogg->fsound, op->packet, op->bytes) ;

	if ((psf->sf.format & SF_FORMAT_SUBMASK) == 0)
		return OGGZ_CONTINUE ;

	return OGGZ_STOP_OK ;
} /* pogg_read_header_packet */

static int
pogg_read_header (SF_PRIVATE *psf)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	unsigned char buf [1024] ;

	OGGZ * oggz ;
	FishSound * fsound ;
	FishSoundInfo fsinfo ;
	int nread = 1024 ;

	psf->sf.format = SF_FORMAT_OGG ;
	psf->sf.frames = 0 ;

	oggz = oggz_new (OGGZ_READ|OGGZ_AUTO) ;

	oggz_io_set_read (oggz, pogg_io_read, psf) ;
	oggz_io_set_seek (oggz, pogg_io_seek, psf) ;
	oggz_io_set_tell (oggz, pogg_io_tell, psf) ;

	fsound = fish_sound_new (FISH_SOUND_DECODE, &fsinfo) ;
	fish_sound_set_interleave (fsound, 1) ;
	fish_sound_set_decoded_callback (fsound, pogg_decode_header, psf) ;

	pogg->oggz = oggz ;
	pogg->fsound = fsound ;
	pogg->serialno = -1 ;
	pogg->cache_pcm = NULL ;
	pogg->cache_size = 0 ;
	pogg->cache_granulepos = 0 ; /* We set this to a known value of zero to begin */
	pogg->cache_frames = 0 ;
	pogg->cache_remaining = 0 ;
	pogg->ptr = NULL ;
	pogg->pcmtype = POGG_PCM_SHORT ;
	pogg->remaining = 0 ;
	pogg->seek_from_start = 0 ;

	/* Set position to start of file to begin reading header. */
	psf_binheader_readf (psf, "p", 0) ;

	/* Get the header info */
	oggz_set_read_callback (oggz, -1, pogg_read_header_packet, psf) ;
	while (nread > 0 && ((psf->sf.format & SF_FORMAT_SUBMASK) == 0))
	{	nread = psf_binheader_readf (psf, "b", buf, sizeof (buf)) ;
		oggz_read_input (oggz, buf, nread) ;
		} ;

	/* Get the duration */
	oggz_set_read_callback (oggz, -1, NULL, NULL) ;
	oggz_set_read_callback (oggz, pogg->serialno, pogg_read_last_packet, psf) ;
	oggz_seek_units (oggz, 0, SEEK_END) ;
	nread = 1024 ;
	while (nread > 0)
		nread = oggz_read (oggz, 1024) ;

	/* reset to the beginning of the audio data */
	oggz_seek_units (oggz, 0, SEEK_SET) ;

	psf->dataoffset = oggz_tell (oggz) ;
	psf->datalength = psf->filelength - psf->dataoffset ;

	/* set the Oggz and FishSound up for decoding */
	oggz_set_read_callback (oggz, -1, NULL, NULL) ;
	oggz_set_read_callback (oggz, pogg->serialno, pogg_read_packet, psf) ;
	fish_sound_set_decoded_callback (fsound, pogg_decode, psf) ;

	return 0 ;
} /* pogg_read_header */

/*------------------------------------------------------------------------------
** Decode functions
*/

static int
pogg_copyout (SF_PRIVATE * psf)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	size_t frame_size, bytes, cache_offset ;
	long cache_usable, i ;
	unsigned char * src ;

	if (pogg->seek_from_start > 0)
	{	/* If we've seeked and don't know where we are, don't do anything yet */
		if (pogg->cache_granulepos == -1)
			return -1 ;

		/* If we've seeked and are before the seek point, don't do anything yet */
		else if (pogg->cache_granulepos < pogg->seek_from_start)
			return -1 ;

		/* If this block contains the seek point, adjust the cache offset accordingly */
		else if (pogg->cache_granulepos - pogg->cache_frames <= pogg->seek_from_start)
		{	pogg->cache_remaining = pogg->cache_granulepos - pogg->seek_from_start ;
			pogg->seek_from_start = 0 ; /* bingo */
			} ;
		} ;

	frame_size = psf->sf.channels * sizeof (float) ;
	cache_usable = SF_MIN (pogg->remaining, pogg->cache_remaining) ;

	if (cache_usable <= 0)
		return 0 ;

	bytes = cache_usable * frame_size ;
	cache_offset = (pogg->cache_frames - pogg->cache_remaining) * frame_size ;
	src = (unsigned char *) pogg->cache_pcm + cache_offset ;

	switch (pogg->pcmtype)
	{	case POGG_PCM_SHORT :
			for (i = 0 ; i < cache_usable ; i++)
				((short *) pogg->ptr) [i] = (short) (((float *) src) [i] * SHRT_MAX) ;
			break ;

		case POGG_PCM_INT :
			for (i = 0 ; i < cache_usable ; i++)
				((double *) pogg->ptr) [i] = (double) (((float *) src) [i] * INT_MAX) ;
			break ;

		case POGG_PCM_FLOAT :
			memcpy (pogg->ptr, src, bytes) ;
			break ;

		case POGG_PCM_DOUBLE :
			for (i = 0 ; i < cache_usable ; i++)
				((double *) pogg->ptr) [i] = (double) ((float *) src) [i] ;
			break ;
		} ;

	pogg->ptr += bytes ;
	pogg->cache_remaining -= cache_usable ;
	pogg->remaining -= cache_usable ;

	return 0 ;
} /* pogg_copyout*/

static int
pogg_decode (FishSound * fsound, float ** pcm, long frames, void * user_data)
{	SF_PRIVATE * psf = (SF_PRIVATE *) user_data ;
	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	size_t bytes ;
	float ** new_block ;

	/* Avoid compiler warning. */
	fsound = NULL ;

	bytes = sizeof (float) * psf->sf.channels * frames ;

	if (bytes > pogg->cache_size)
	{	new_block = realloc (pogg->cache_pcm, bytes) ;
		if (new_block == NULL)
			/* XXX : SFE_MALLOC_FAILED */
			return -1 ;

		pogg->cache_pcm = new_block ;
		pogg->cache_size = bytes ;
		} ;

	memcpy (pogg->cache_pcm, pcm, bytes) ;
	pogg->cache_frames = frames ;
	pogg->cache_remaining = frames ;

	if (pogg->cache_granulepos != -1)
		pogg->cache_granulepos += frames ;

	pogg_copyout (psf) ;

	return 0 ;
} /* pogg_decode */

static int
pogg_read_packet (OGGZ * oggz, ogg_packet * op, long serialno, void * data)
{	SF_PRIVATE * psf = (SF_PRIVATE *) data ;
	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	FishSound * fsound = pogg->fsound ;

	/* Avoid warning message. */
	oggz = NULL ;
	serialno = 0 ;

	fish_sound_decode (fsound, op->packet, op->bytes) ;

	if (op->granulepos != -1)
		pogg->cache_granulepos = op->granulepos ;

	if (pogg->remaining == 0)
		return OGGZ_STOP_OK ;

	return OGGZ_CONTINUE ;
} /* pogg_read_packet */

static sf_count_t
pogg_read_loop (SF_PRIVATE *psf, sf_count_t len)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	long nread = 1024 ;
	sf_count_t ret = len ;

	/** Calculate nr. frames remaining */
	pogg->remaining = len / psf->sf.channels ;

	/** Serve out any remaining cached data first */
	pogg_copyout (psf) ;

	while (nread > 0 && pogg->remaining > 0)
		nread = oggz_read (pogg->oggz, 1024) ;

	if (nread == 0)
		ret -= pogg->remaining * psf->sf.channels ;

	return ret ;
} /* pogg_read_loop */

static sf_count_t
pogg_read_s (SF_PRIVATE *psf, short *ptr, sf_count_t len)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;

	pogg->ptr = ptr ;
	pogg->pcmtype = POGG_PCM_SHORT ;

	return pogg_read_loop (psf, len) ;
} /* pogg_read_s */

static sf_count_t
pogg_read_i (SF_PRIVATE *psf, int *ptr, sf_count_t len)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;

	pogg->ptr = ptr ;
	pogg->pcmtype = POGG_PCM_INT ;

	return pogg_read_loop (psf, len) ;
} /* pogg_read_i */

static sf_count_t
pogg_read_f (SF_PRIVATE *psf, float *ptr, sf_count_t len)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;

	pogg->ptr = ptr ;
	pogg->pcmtype = POGG_PCM_FLOAT ;

	return pogg_read_loop (psf, len) ;
} /* pogg_read_f */

static sf_count_t
pogg_read_d (SF_PRIVATE *psf, double *ptr, sf_count_t len)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;

	pogg->ptr = ptr ;
	pogg->pcmtype = POGG_PCM_DOUBLE ;

	return pogg_read_loop (psf, len) ;
} /* pogg_read_d */

/*------------------------------------------------------------------------------
** Seek functions
*/

static sf_count_t
pogg_seek (SF_PRIVATE *psf, int mode, sf_count_t seek_from_start)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	ogg_int64_t units = seek_from_start * 1000 / psf->sf.samplerate ;

	if (mode != SFM_READ)
	{	psf->error = SFE_BAD_SEEK ;
		return SF_SEEK_ERROR ;
		} ;

	oggz_seek_units (pogg->oggz, units, SEEK_SET) ;

	/* Invalidate cache and set the desired seek position */
	pogg->cache_remaining = 0 ;
	pogg->cache_granulepos = -1 ;
	pogg->seek_from_start = seek_from_start ;

	return seek_from_start ;
} /* pogg_seek */

/*------------------------------------------------------------------------------
** Write functions
*/

static int
pogg_encoded (FishSound * fsound, unsigned char * buf, long bytes, void * user_data)
{	SF_PRIVATE * psf = (SF_PRIVATE *) user_data ;
	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	ogg_packet op ;
	int err ;

	op.packet = buf ;
	op.bytes = bytes ;
	op.b_o_s = pogg->b_o_s ;
	op.e_o_s = 0 ;
	op.granulepos = fish_sound_get_frameno (fsound) ;
	op.packetno = -1 ;

	err = oggz_write_feed (pogg->oggz, &op, pogg->serialno, 0, NULL) ;

	pogg->b_o_s = 0 ;

	return OGGZ_CONTINUE ;
} /* pogg_encoded */

static int
pogg_write_anx_headers (SF_PRIVATE *psf, int format)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	unsigned char buf [128] ;
	long anx_serialno ;
	int headers_len = 0 ;
	const char * content_type ;
	ogg_packet op ;
	int err ;

	anx_serialno = oggz_serialno_new (pogg->oggz) ;

	/* Write Annodex header */
	memset (buf, 0, 48) ;
	snprintf (buf, 8, "Annodex") ;

	/* Version */
	*(ogg_int16_t *) &buf [8] = (ogg_int16_t) POGG_ANX_VERSION_MAJOR ;
	*(ogg_int16_t *) &buf [10] = (ogg_int16_t) POGG_ANX_VERSION_MINOR ;
	if (CPU_IS_BIG_ENDIAN)
		endswap_short_array ((short *) &buf [8], 2) ;

	/* Timebase numerator */
	*(ogg_int64_t *) &buf [12] = (ogg_int64_t) 0 ;

	/* Timebase denominator */
	*(ogg_int64_t *) &buf [20] = (ogg_int64_t) 1 ;

	if (CPU_IS_BIG_ENDIAN)
		endswap_long_array ((long *) &buf [12], 2) ;

	op.packet = buf ;
	op.bytes = 48 ;
	op.b_o_s = 1 ;
	op.e_o_s = 0 ;
	op.granulepos = 0 ;
	op.packetno = -1 ;

	err = oggz_write_feed (pogg->oggz, &op, anx_serialno, 0, NULL) ;

	pogg->b_o_s = 0 ;

	/* Write AnxData header */
	memset (buf, 0, 48) ;
	snprintf (buf, 8, "AnxData") ;

	/* Granule rate numerator */
	*(ogg_int64_t *) &buf [8] = (ogg_int64_t) psf->sf.samplerate ;

	/* Granule rate denominator */
	*(ogg_int64_t *) &buf [16] = (ogg_int64_t) 1 ;

	if (CPU_IS_BIG_ENDIAN)
		endswap_long_array ((long *) &buf [8], 2) ;

	/* Number of secondary header pages */
	*(ogg_int32_t *) &buf [24] = (ogg_int32_t) 3 ;
	if (CPU_IS_BIG_ENDIAN)
		endswap_int_array ((int *) &buf [24], 1) ;

	/* Headers */
	if (format == FISH_SOUND_VORBIS)
		content_type = POGG_VORBIS_CONTENT_TYPE ;
	else
		content_type = POGG_SPEEX_CONTENT_TYPE ;

	headers_len = snprintf ((char *) &buf [28], 100, "Content-Type : %s\r\n", content_type) ;

	op.packet = buf ;
	op.bytes = 28 + headers_len + 1 ;
	op.b_o_s = 1 ;
	op.e_o_s = 0 ;
	op.granulepos = 0 ;
	op.packetno = -1 ;

	err = oggz_write_feed (pogg->oggz, &op, pogg->serialno, 0, NULL) ;

	/* Write Annodex eos packet */
	op.packet = NULL ;
	op.bytes = 0 ;
	op.b_o_s = 0 ;
	op.e_o_s = 1 ;
	op.granulepos = 0 ;
	op.packetno = -1 ;

	err = oggz_write_feed (pogg->oggz, &op, anx_serialno, 0, NULL) ;

	return 0 ;
} /* pogg_write_anx_headers */

static int
pogg_write_header (SF_PRIVATE *psf)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;

	OGGZ * oggz ;
	FishSound * fsound ;
	FishSoundInfo fsinfo ;

	oggz = oggz_new (OGGZ_WRITE) ;

	oggz_io_set_write (oggz, pogg_io_write, psf) ;

	fsinfo.samplerate = psf->sf.samplerate ;
	fsinfo.channels = psf->sf.channels ;

	switch (psf->sf.format & SF_FORMAT_SUBMASK)
	{	case SF_FORMAT_VORBIS :
			fsinfo.format = FISH_SOUND_VORBIS ;
		break ;

		case SF_FORMAT_SPEEX :
			fsinfo.format = FISH_SOUND_SPEEX ;
			break ;
		} ;

	fsound = fish_sound_new (FISH_SOUND_ENCODE, &fsinfo) ;
	fish_sound_set_interleave (fsound, 1) ;
	fish_sound_set_encoded_callback (fsound, pogg_encoded, psf) ;

	pogg->oggz = oggz ;
	pogg->fsound = fsound ;
	pogg->serialno = oggz_serialno_new (oggz) ;
	pogg->b_o_s = 1 ;
	pogg->comments_written = 0 ;
	pogg->granulepos = 0 ;
	pogg->fptr = (float *) malloc (sizeof (float) * 1024) ;

	if ((psf->sf.format & SF_FORMAT_TYPEMASK) == SF_FORMAT_ANX)
		pogg_write_anx_headers (psf, fsinfo.format) ;

	return 0 ;
} /* pogg_write_header */

static void
pogg_write_comments (SF_PRIVATE *psf)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	int k ;
	int err ;

	for (k = 0 ; k < SF_MAX_STRINGS ; k++)
	{	printf ("hrumf comment : %d => %s\n", psf->strings [k].type, psf->strings [k].str) ;
		/*
			if (psf->strings [k].type == 0)
				break ;
		*/

		if (psf->strings [k].type != 0)
		{	printf ("adding comment : %d => %s\n", psf->strings [k].type, psf->strings [k].str) ;

			switch (psf->strings [k].type)
			{	case SF_STR_SOFTWARE :
					err = fish_sound_comment_add_byname (pogg->fsound, "ENCODER", psf->strings [k].str) ;
					break ;

				case SF_STR_TITLE :
					err = fish_sound_comment_add_byname (pogg->fsound, "TITLE", psf->strings [k].str) ;
					break ;

				case SF_STR_COPYRIGHT :
					err = fish_sound_comment_add_byname (pogg->fsound, "COPYRIGHT", psf->strings [k].str) ;
					break ;

				case SF_STR_ARTIST :
					err = fish_sound_comment_add_byname (pogg->fsound, "ARTIST", psf->strings [k].str) ;
					break ;

				/*
				case SF_STR_COMMENT :
					fish_sound_comment_add_byname (pogg->fsound, "COMMENT", psf->strings [k].str) ;
					break ;
				*/

				case SF_STR_DATE :
					err = fish_sound_comment_add_byname (pogg->fsound, "DATE", psf->strings [k].str) ;
					break ;
				} ;

			} ; /* if type !0 */
		} ;

	pogg->comments_written = 1 ;

	return ;
} /* pogg_write_comments*/

static sf_count_t
pogg_write_s (SF_PRIVATE *psf, short *ptr, sf_count_t len)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	long n, i, remaining = len / psf->sf.channels ;

	if (pogg->comments_written == 0)
		pogg_write_comments (psf) ;

	while (remaining > 0)
	{	 n = SF_MIN (remaining, 1024) ;

		for (i = 0 ; i < n * psf->sf.channels ; i++)
			pogg->fptr [i] = (float) ptr [i] ;

		fish_sound_encode (pogg->fsound, (float **) ptr, n) ;

		while (oggz_write (pogg->oggz, 1024) > 0) ;

		ptr += n * psf->sf.channels ;
		remaining -= n ;
		} ;

	return len ;
} /* pogg_write_s */

static sf_count_t
pogg_write_i (SF_PRIVATE *psf, int *ptr, sf_count_t len)
{
	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	long n, i, remaining = len / psf->sf.channels ;

	if (pogg->comments_written == 0)
		pogg_write_comments (psf) ;

	while (remaining > 0)
	{	n = SF_MIN (remaining, 1024) ;

		for (i = 0 ; i < n * psf->sf.channels ; i++)
			pogg->fptr [i] = (float) ptr [i] ;

		fish_sound_encode (pogg->fsound, (float **) ptr, n) ;

		while (oggz_write (pogg->oggz, 1024) > 0) ;

		ptr += n * psf->sf.channels ;
		remaining -= n ;
		} ;

	return len ;
} /* pogg_write_i */

static sf_count_t
pogg_write_f (SF_PRIVATE *psf, float *ptr, sf_count_t len)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	long n, remaining = len / psf->sf.channels ;

	if (pogg->comments_written == 0)
		pogg_write_comments (psf) ;

	while (remaining > 0)
	{	n = SF_MIN (remaining, 1024) ;

		fish_sound_encode (pogg->fsound, (float **) ptr, n) ;

		while (oggz_write (pogg->oggz, 1024) > 0) ;

		ptr += n * psf->sf.channels ;
		remaining -= n ;
		} ;

	return len ;
} /* pog_write_f */

static sf_count_t
pogg_write_d (SF_PRIVATE *psf, double *ptr, sf_count_t len)
{	OGG_PRIVATE * pogg = (OGG_PRIVATE *) psf->fdata ;
	long n, i, remaining = len / psf->sf.channels ;

	if (pogg->comments_written == 0)
		pogg_write_comments (psf) ;

	while (remaining > 0)
	{	n = SF_MIN (remaining, 1024) ;

		for (i = 0 ; i < n * psf->sf.channels ; i++)
			pogg->fptr [i] = (float) ptr [i] ;

		fish_sound_encode (pogg->fsound, (float **) ptr, n) ;

		while (oggz_write (pogg->oggz, 1024) > 0) ;

		ptr += n * psf->sf.channels ;
		remaining -= n ;
		} ;

	return len ;
} /* pogg_write_d */

#endif

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: 9ff1fe9c-629e-4e9c-9ef5-3d0eb1e427a0
*/
