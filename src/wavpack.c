/*
** Copyright (C) 2002-2011 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (C) 2020 tuxzz <tuku@tuxzz.org>
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



#include	"sfconfig.h"

#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<ctype.h>
#include	<math.h>

#include	"sndfile.h"
#include	"sfendian.h"
#include	"common.h"
#include	"wavlike.h"

#if HAVE_WAVPACK

#include	<wavpack/wavpack.h>

/*------------------------------------------------------------------------------
** Macros
*/

#define WAVPACK_GENERATE_FLOAT_TO_INT_FUNC(FUNC_NAME, ITYPE, FTYPE, LRFUNC, MAXFUNC, MINFUNC) \
static void \
FUNC_NAME (const FTYPE *src, ITYPE *dest, sf_count_t count, int normalize, FTYPE vlim, ITYPE offset, int use_clip) \
{	double normfact = normalize ? ((double) vlim) : 1.0 ; \
	sf_count_t i ; \
	if ((use_clip)) \
	{	for (i = 0 ; i < count ; ++ i) \
			dest [i] = lrint (fmax (- ((double) vlim), fmin (((double) src [i]) * normfact, (((double) vlim) - 1.0)))) + (offset) ; \
		} \
	else \
	{	for (i = 0 ; i < count ; ++ i) \
			dest [i] = lrint (((double) src [i]) * normfact) + (offset) ; \
		} ; \
} /* FUNC_NAME */

#define WAVPACK_GENERATE_FLOAT_TO_INT_FUNC_INPLACE(FUNC_NAME, ITYPE, FTYPE, LRFUNC, MAXFUNC, MINFUNC) \
static void \
FUNC_NAME (void *src_dest, sf_count_t count, int normalize, FTYPE vlim, ITYPE offset, int use_clip) \
{	double normfact = normalize ? ((double) vlim) : 1.0 ; \
	sf_count_t i ; \
	if ((use_clip)) \
	{	for (i = 0 ; i < count ; ++ i) \
			* (((ITYPE *) src_dest) + i) = lrint (fmax (- ((double) vlim), fmin (((double) (* (((const FTYPE *) src_dest) + i))) * normfact, (((double) vlim) - 1.0)))) + (offset) ; \
		} \
	else \
	{	for (i = 0 ; i < count ; ++ i) \
			* (((ITYPE *) src_dest) + i) = lrint (((double) (* (((const FTYPE *) src_dest) + i))) * normfact) + (offset) ; \
		} ; \
} /* FUNC_NAME */

#define WAVPACK_GENERATE_INT_TO_FLOAT_FUNC(FUNC_NAME, ITYPE, FTYPE, VLIM, OFFSET) \
static void \
FUNC_NAME (const ITYPE *src, FTYPE *dest, sf_count_t count, int normalize) \
{	FTYPE normfact = normalize ? (VLIM) : 1.0 ; \
	sf_count_t i ; \
	for (i = 0 ; i < count ; ++ i) \
		dest [i] = (((FTYPE) src [i]) - (OFFSET)) / normfact ; \
} /* FUNC_NAME */

#define WAVPACK_GENERATE_INT_TO_FLOAT_FUNC_INPLACE(FUNC_NAME, ITYPE, FTYPE, VLIM, OFFSET) \
static void \
FUNC_NAME (void *src_dest, sf_count_t count, int normalize) \
{	FTYPE normfact = normalize ? (VLIM) : 1.0 ; \
	sf_count_t i ; \
	for (i = 0 ; i < count ; ++ i) \
		* (((FTYPE *) src_dest) + i) = (((FTYPE) (* (((const ITYPE *) src_dest) + i))) - (OFFSET)) / normfact ; \
} /* FUNC_NAME */

#define WAVPACK_GENERATE_SHIFT_FUNC(FUNC_NAME, ITYPE, OTYPE) \
static void \
FUNC_NAME (const ITYPE *src, OTYPE *dest, sf_count_t count, int shl, int offset) \
{	sf_count_t i ; \
	if (shl == 0 && offset == 0) \
	{	for (i = 0 ; i < count ; ++ i) \
			dest [i] = (OTYPE) src [i] ; \
		} \
	else if (shl >= 0) \
	{	for (i = 0 ; i < count ; ++ i) \
			dest [i] = (((OTYPE) src [i]) * (((OTYPE) 1) << shl)) + ((OTYPE) offset) ; \
		} \
	else \
	{	for (i = 0 ; i < count ; ++ i) \
			dest [i] = (OTYPE) ((src [i] / (((ITYPE) 1) << (-shl))) + ((ITYPE) offset)) ; \
		} ; \
} /* FUNC_NAME */

/*------------------------------------------------------------------------------
** Typedefs.
*/

typedef struct
{	WavpackConfig config ;
	WavpackContext *context ;
	void *buffer ;
	void *first_block ;
	int64_t first_pos ;
	int32_t first_bcount ;
	int64_t sequential_pos ;
	int is_seek_end, is_in_close, is_random_access_device, is_header_written ;
	unsigned char last_char, last_char_status ;
} WAVPACK_PRIVATE ;

typedef enum
{	LAST_CHAR_INVALID = 0,
	LAST_CHAR_VALID
} LAST_CHAR_STATUS ;

/*------------------------------------------------------------------------------
** Private static functions and variables.
*/

#define WAVPACK_BUFFER_LENGTH 131072

#define WVPK_UNPACK_FAST_PATH_MAX_SAMPLES 0x7fffffff
#define WVPK_INT32_VLIM 0x80000000
#define WVPK_INT24_VLIM 0x800000
#define WVPK_INT16_VLIM 0x8000
#define WVPK_INT8_VLIM 0x80

WAVPACK_GENERATE_FLOAT_TO_INT_FUNC (wavpack_cvtf32i32, int32_t, float, lrintf, fmaxf, fminf)
WAVPACK_GENERATE_FLOAT_TO_INT_FUNC (wavpack_cvtf64i32, int32_t, double, lrint, fmax, fmin)
WAVPACK_GENERATE_FLOAT_TO_INT_FUNC (wavpack_cvtf32i16, int16_t, float, lrintf, fmaxf, fminf)

WAVPACK_GENERATE_FLOAT_TO_INT_FUNC_INPLACE (wavpack_cvtf32i32_inplace, int32_t, float, lrintf, fmaxf, fminf)

WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvti16f32, short, float, WVPK_INT16_VLIM, 0)

WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvt32i8f32, int32_t, float, WVPK_INT8_VLIM, 0)
WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvt32i16f32, int32_t, float, WVPK_INT16_VLIM, 0)
WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvt32i24f32, int32_t, float, WVPK_INT24_VLIM, 0)
WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvti32f32, int32_t, float, WVPK_INT32_VLIM, 0)

WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvt32i8f64, int32_t, double, WVPK_INT8_VLIM, 0)
WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvt32i16f64, int32_t, double, WVPK_INT16_VLIM, 0)
WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvt32i24f64, int32_t, double, WVPK_INT24_VLIM, 0)
WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvti32f64, int32_t, double, WVPK_INT32_VLIM, 0)

WAVPACK_GENERATE_INT_TO_FLOAT_FUNC_INPLACE (wavpack_cvti32f32_inplace, int32_t, float, WVPK_INT32_VLIM, 0)

WAVPACK_GENERATE_SHIFT_FUNC (wavpack_shifti32i16, int32_t, int16_t) ;
WAVPACK_GENERATE_SHIFT_FUNC (wavpack_shifti16i32, int16_t, int32_t) ;
WAVPACK_GENERATE_SHIFT_FUNC (wavpack_shifti32i32, int32_t, int32_t) ;


static void		wavpack_cvtf64f32 (const double *src, float *dest, sf_count_t count) ;
static void		wavpack_cvtf32f64 (const float *src, double *dest, sf_count_t count) ;

static inline int	wavpack_pack_buffer_single_any
(	SF_PRIVATE *psf, sf_count_t *total_packed_count, const void *ptr, sf_count_t i_chunk, sf_count_t chunk_item_count, sf_count_t item_count, int32_t *buffer,
	int itype, int otype, float vlim, int shl, int voffset
) ;
static inline int	wavpack_unpack_buffer_single_any
(	SF_PRIVATE *psf, sf_count_t *total_packed_count, void *ptr, sf_count_t i_chunk, sf_count_t chunk_item_count, sf_count_t item_count, int32_t *buffer,
	int itype, int otype, float vlim, int shl, int voffset
) ;

static sf_count_t	wavpack_unpack_buffer_any (SF_PRIVATE *psf, void *ptr, sf_count_t sample_count, int otype) ;
static sf_count_t	wavpack_pack_buffer_any (SF_PRIVATE *psf, const void *ptr, sf_count_t sample_count, int itype) ;

static int 		wavpack_enc_init (SF_PRIVATE *psf) ;
static int 		wavpack_dec_init (SF_PRIVATE *psf) ;
static int		wavpack_read_strings (SF_PRIVATE *psf) ;
static int		wavpack_write_strings (SF_PRIVATE *psf) ;
static int		wavpack_write_header (SF_PRIVATE *psf, int UNUSED (calc_length)) ;
static int		wavpack_command (SF_PRIVATE *psf, int command, void *UNUSED (data), int UNUSED (datasize)) ;
static int 		wavpack_close (SF_PRIVATE *psf) ;
static sf_count_t	wavpack_seek (SF_PRIVATE *psf, int UNUSED (mode), sf_count_t offset) ;
static int		wavpack_byterate (SF_PRIVATE *psf) ;

static sf_count_t		wavpack_read_short (SF_PRIVATE *psf, short *ptr, sf_count_t len) ;
static sf_count_t		wavpack_read_int (SF_PRIVATE *psf, int *ptr, sf_count_t len) ;
static sf_count_t		wavpack_read_float (SF_PRIVATE *psf, float *ptr, sf_count_t len) ;
static sf_count_t		wavpack_read_double (SF_PRIVATE *psf, double *ptr, sf_count_t len) ;

static sf_count_t		wavpack_write_short (SF_PRIVATE *psf, const short *ptr, sf_count_t len) ;
static sf_count_t		wavpack_write_int (SF_PRIVATE *psf, const int *ptr, sf_count_t len) ;
static sf_count_t		wavpack_write_float (SF_PRIVATE *psf, const float *ptr, sf_count_t len) ;
static sf_count_t		wavpack_write_double (SF_PRIVATE *psf, const double *ptr, sf_count_t len) ;

static int32_t	sf_wavpack_read_bytes_callback (void *client_data, void *data, int32_t bcount) ;
static int32_t	sf_wavpack_write_bytes_callback (void *client_data, void *data, int32_t bcount) ;
static int64_t	sf_wavpack_get_pos_callback (void *client_data) ;
static int		sf_wavpack_set_pos_abs_callback (void *client_data, int64_t pos) ;
static int		sf_wavpack_set_pos_rel_callback (void *client_data, int64_t delta, int mode) ;
static int		sf_wavpack_push_back_byte_callback (void *id, int c) ;
static int64_t	sf_wavpack_get_length_callback (void *client_data) ;
static int		sf_wavpack_can_seek_callback (void *client_data) ;
static int		sf_wavpack_truncate_here (void *client_data) ;

static int 		wavpack_io_feature_test (SF_PRIVATE *psf) ;

static WavpackStreamReader64	wavpack_reader_wrapper =
{	sf_wavpack_read_bytes_callback,
	sf_wavpack_write_bytes_callback,
	sf_wavpack_get_pos_callback,
	sf_wavpack_set_pos_abs_callback,
	sf_wavpack_set_pos_rel_callback,
	sf_wavpack_push_back_byte_callback,
	sf_wavpack_get_length_callback,
	sf_wavpack_can_seek_callback,
	sf_wavpack_truncate_here,
	NULL
} ;

/*------------------------------------------------------------------------------
** Public function.
*/

int
wavpack_open	(SF_PRIVATE *psf)
{	// int		subformat ;
	int		error = 0 ;

	if (psf->file.mode == SFM_RDWR)
		return SFE_BAD_MODE_RW ;

	WAVPACK_PRIVATE* pwvpk = calloc (1, sizeof (WAVPACK_PRIVATE)) ;
	if (pwvpk == NULL)
		return SFE_MALLOC_FAILED ;

	psf->codec_data = pwvpk ;
	psf->dataoffset = 0 ;
	psf->datalength = psf->filelength ;

	pwvpk->is_random_access_device = wavpack_io_feature_test (psf) ;
	if (pwvpk->is_random_access_device < 0)
	{	psf_log_printf (psf, "wavpack_open: failed to call `wavpack_io_feature_test`, err=%d", pwvpk->is_random_access_device) ;
		error = SFE_OPEN_FAILED ;
		goto on_error ;
		} ;

	if (psf->file.mode == SFM_READ)
	{	psf->sf.seekable = pwvpk->is_random_access_device ;
		if ((error = wavpack_dec_init (psf)))
		{	psf_log_printf (psf, "wavpack_open: failed to call `wavpack_dec_init`, error is `%s`(%d)\n", sf_strerror ((SNDFILE*) psf), psf->error) ;
			goto on_error ;
			} ;
		} ;

	if (psf->file.mode == SFM_WRITE)
	{	if ((SF_CONTAINER (psf->sf.format)) != SF_FORMAT_WAVPACK)
		{	psf_log_printf (psf, "wavpack_open: invalid container in `psf->sf.format`, got %d(raw=%d)\n", SF_CONTAINER (psf->sf.format), psf->sf.format) ;
			error = SFE_BAD_OPEN_FORMAT ;
			goto on_error ;
			} ;

		psf->endian = SF_ENDIAN_LITTLE ;
		psf->sf.seekable = 0 ;

		psf->strings.flags = SF_STR_ALLOW_START | SF_STR_ALLOW_END ;

		if ((error = wavpack_enc_init (psf)))
		{	psf_log_printf (psf, "wavpack_open: failed to call `wavpack_enc_init`, error is `%s`(%d)\n", sf_strerror ((SNDFILE*) psf), psf->error) ;
			goto on_error ;
			} ;

		/* In an ideal world we would write the header at this point. Unfortunately
		** that would prevent channel mask or string metadata being added so we have to hold off.
		*/
		psf->write_header = wavpack_write_header ;
		} ;

	if (psf->filelength > psf->dataoffset)
		psf->datalength = (psf->dataend) ? psf->dataend - psf->dataoffset : psf->filelength - psf->dataoffset ;
	else
		psf->datalength = 0 ;

	psf->container_close = wavpack_close ;
	psf->seek = wavpack_seek ;
	psf->byterate = wavpack_byterate ;

	psf->command = wavpack_command ;

	if (psf->file.mode == SFM_READ)
	{	psf->read_short		= wavpack_read_short ;
		psf->read_int		= wavpack_read_int ;
		psf->read_float		= wavpack_read_float ;
		psf->read_double	= wavpack_read_double ;
		} ;

	if (psf->file.mode == SFM_WRITE)
	{	psf->write_short	= wavpack_write_short ;
		psf->write_int		= wavpack_write_int ;
		psf->write_float	= wavpack_write_float ;
		psf->write_double	= wavpack_write_double ;
		} ;

	return SFE_NO_ERROR ;

on_error:
	free (pwvpk) ;
	psf->codec_data = NULL ;
	return error ;
} /* wavpack_open */

/*------------------------------------------------------------------------------
*/

static int
wavpack_command (SF_PRIVATE *psf, int command, void *UNUSED (data), int UNUSED (datasize))
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	if (pwvpk == NULL)
	{	psf_log_printf (psf, "wavpack_command: null pwvpk\n") ;
		return SFE_INTERNAL ;
		} ;

	if (pwvpk->context == NULL)
	{	psf_log_printf (psf, "wavpack_command: null pwvpk->context\n") ;
		return SFE_INTERNAL ;
		} ;

	switch (command)
	{	case SFC_SET_CHANNEL_MAP_INFO :
			pwvpk->config.channel_mask = wavlike_gen_channel_mask (psf->channel_map, psf->sf.channels) ;
			return (pwvpk->config.channel_mask != 0) ;

		default :
			break ;
		} ;

	return 0 ;
} /* wavpack_command */

static int
wavpack_read_header (SF_PRIVATE *psf)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	int err = SFE_NO_ERROR ;
	if (pwvpk == NULL)
	{	psf_log_printf (psf, "wavpack_read_header: null pwvpk\n") ;
		return SFE_INTERNAL ;
		} ;

	if (pwvpk->context == NULL)
	{	psf_log_printf (psf, "wavpack_read_header: null pwvpk->context\n") ;
		return SFE_INTERNAL ;
		} ;

	// now let's read header
	psf->sf.samplerate = WavpackGetSampleRate (pwvpk->context) ;
	psf->channel_map = wavlike_gen_channel_map (WavpackGetChannelMask (pwvpk->context)) ;
	psf->sf.channels = WavpackGetNumChannels (pwvpk->context) ;
	if ((psf->sf.frames = WavpackGetNumSamples64 (pwvpk->context)) < 0)
		psf->sf.frames = 0 ;

	uint32_t bit_depth = WavpackGetBitsPerSample (pwvpk->context) ;
	int qmode = WavpackGetQualifyMode (pwvpk->context) ;
	int mode = WavpackGetMode (pwvpk->context) ;

	if (bit_depth == 8)
	{	if (qmode & QMODE_SIGNED_BYTES)
			psf->sf.format |= SF_FORMAT_PCM_S8 ;
		else
 			psf->sf.format |= SF_FORMAT_PCM_U8 ;
		}
	else if (qmode & QMODE_UNSIGNED_WORDS)
	{	psf_log_printf (psf, "wavpack_read_header: `bit_depth = %u, qmode = %d` not implemented\n", bit_depth, qmode) ;
		return SFE_UNIMPLEMENTED ;
		}
	else if (mode & MODE_FLOAT)
		psf->sf.format |= SF_FORMAT_FLOAT ;
	else
	{	switch ((bit_depth + 7) & 0x38)     // round up bitdepth to handle e.g. 20-bit files
		{	case 16 :
				psf->sf.format |= SF_FORMAT_PCM_16 ;
				break ;
			case 24 :
				psf->sf.format |= SF_FORMAT_PCM_24 ;
				break ;
			case 32 :
				psf->sf.format |= SF_FORMAT_PCM_32 ;
				break ;
			default :
				psf_log_printf (psf, "wavpack_read_header: `bit_depth = %u, qmode = %d` not implemented\n", bit_depth, qmode) ;
				return SFE_UNIMPLEMENTED ;
			} ;
		} ;

	if (pwvpk->is_random_access_device && (err = wavpack_read_strings (psf)) != SFE_NO_ERROR)
	{	psf_log_printf (psf, "wavpack_read_header: failed to call `wavpack_read_strings`.\n") ;
		return err ;
		} ;

	return SFE_NO_ERROR ;
}

static int
wavpack_dec_init (SF_PRIVATE *psf)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	char error_buf [128] = { 0 } ;
	int err ;

	if (pwvpk->context)
	{	pwvpk->context = WavpackCloseFile (pwvpk->context) ;
		psf_log_printf (psf, "wavpack_dec_init: reinitialization for decoder is not unexpected, this is a bug.\n") ;
		return SFE_INTERNAL ;
		} ;

	if (pwvpk->is_random_access_device)
	{	psf_fseek (psf, 0, SEEK_SET) ;
		if (psf->error != SFE_NO_ERROR)
		{	psf_log_printf (psf, "wavpack_dec_init: failed to call `psf_fseek`, error is `%s`(%d)\n", sf_strerror ((SNDFILE*) psf), psf->error) ;
			return psf->error ;
			} ;
		} ;

        /* adjust float files that are [very rarely] not nomalized to +/- 1.0 (e.g., +/- 32768.0) */
        /* also allow opening DSD files as 24-bit PCM (decimated 8x) */
        pwvpk->config.flags = OPEN_NORMALIZE | OPEN_DSD_AS_PCM ;

	/* tags in wavpack always at the end of file */
	if (pwvpk->is_random_access_device)
		pwvpk->config.flags |= OPEN_TAGS ;

	if ((pwvpk->context = WavpackOpenFileInputEx64 (&wavpack_reader_wrapper, psf, NULL, error_buf, pwvpk->config.flags, 0)) == NULL)
	{	psf_log_printf (psf, "wavpack_dec_init: failed to call `WavpackOpenFileInputEx64`: `%s`\n", error_buf) ;
		return SFE_UNSUPPORTED_ENCODING ;
		} ;

	if ((err = wavpack_read_header (psf)) != SFE_NO_ERROR)
	{	psf_log_printf (psf, "wavpack_dec_init: failed to call `wavpack_read_header`, err is %d\n", err) ;
		pwvpk->context = WavpackCloseFile (pwvpk->context) ;
		return err ;
		} ;

	return SFE_NO_ERROR ;
} /* wavpack_dec_init */

static int
wavpack_enc_init (SF_PRIVATE *psf)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;

	if (pwvpk->context)
		pwvpk->context = WavpackCloseFile (pwvpk->context) ;
	if (pwvpk->first_block)
	{	free (pwvpk->first_block) ;
		pwvpk->first_block = NULL ;
		} ;

	if (pwvpk->is_random_access_device)
	{	psf_fseek (psf, 0, SEEK_SET) ;
		if (psf->error != SFE_NO_ERROR)
		{	psf_log_printf (psf, "wavpack_enc_init: failed to call `psf_fseek`, error is `%s`(%d)\n", sf_strerror ((SNDFILE*) psf), psf->error) ;
			return psf->error ;
			} ;
		} ;

	pwvpk->config.num_channels = psf->sf.channels ;
	pwvpk->config.sample_rate = psf->sf.samplerate ;
	if (pwvpk->config.num_channels <= 0)
		return SFE_CHANNEL_COUNT_BAD ;
	if (pwvpk->config.sample_rate <= 0)
		return SFE_UNSUPPORTED_ENCODING ;

	switch (psf->sf.channels)
	{	case 1 :	/* center channel mono */
			pwvpk->config.channel_mask = 0x4 ;
			break ;
		case 2 :	/* front left and right */
			pwvpk->config.channel_mask = 0x1 | 0x2 ;
			break ;
		case 4 :	/* Quad */
			pwvpk->config.channel_mask = 0x1 | 0x2 | 0x10 | 0x20 ;
			break ;
		case 6 :	/* 5.1 */
			pwvpk->config.channel_mask = 0x1 | 0x2 | 0x4 | 0x8 | 0x10 | 0x20 ;
			break ;
		case 8 :	/* 7.1 */
			pwvpk->config.channel_mask = 0x1 | 0x2 | 0x4 | 0x8 | 0x10 | 0x20 | 0x40 | 0x80 ;
			break ;
		default :	/* 0 when in doubt, use direct out, i.e. NO mapping */
			pwvpk->config.channel_mask = 0x0 ;
			break ;
		} ;
	switch (SF_CODEC (psf->sf.format))
	{	case SF_FORMAT_PCM_S8 :
			pwvpk->config.bytes_per_sample = 1 ;
			pwvpk->config.bits_per_sample = 8 ;
			pwvpk->config.qmode |= QMODE_SIGNED_BYTES ;
			break ;
		case SF_FORMAT_PCM_U8 :
			pwvpk->config.bytes_per_sample = 1 ;
			pwvpk->config.bits_per_sample = 8 ;
			break ;
		case SF_FORMAT_PCM_16 :
			pwvpk->config.bytes_per_sample = 2 ;
			pwvpk->config.bits_per_sample = 16 ;
			break ;
		case SF_FORMAT_PCM_24 :
			pwvpk->config.bytes_per_sample = 3 ;
			pwvpk->config.bits_per_sample = 24 ;
			break ;
		case SF_FORMAT_PCM_32 :
			pwvpk->config.bytes_per_sample = 4 ;
			pwvpk->config.bits_per_sample = 32 ;
			break ;
		case SF_FORMAT_FLOAT :
			pwvpk->config.bytes_per_sample = 4 ;
			pwvpk->config.bits_per_sample = 32 ;
			pwvpk->config.float_norm_exp = 127 ;
			break ;
		default :
			return SFE_UNIMPLEMENTED ;
		} ;

	if ((pwvpk->context = WavpackOpenFileOutput (wavpack_reader_wrapper.write_bytes, psf, NULL)) == NULL)
	{	psf_log_printf (psf, "wavpack_enc_init: failed to call `WavpackOpenFileOutput`\n") ;
		return SFE_UNSUPPORTED_ENCODING ;
		} ;

	return SFE_NO_ERROR ;
} /* wavpack_enc_init */

static int
wavpack_read_strings (SF_PRIVATE *psf)
{	static const struct
	{	int sf_tag_type ;
		const char *key ;
	} tag_key_table [] =
	{	{ SF_STR_TITLE, "title" },
		{ SF_STR_ARTIST, "artist" },
		{ SF_STR_ALBUM, "album" },
		{ SF_STR_DATE, "year" },
		{ SF_STR_GENRE, "genre" },
		{ SF_STR_TRACKNUMBER, "track" },
		{ SF_STR_COMMENT, "comment" },
		{ SF_STR_COPYRIGHT, "copyright" },
		{ SF_STR_LICENSE, "license" },
		{ SF_STR_SOFTWARE, "software" }
		} ;

	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	if (pwvpk == NULL)
	{	psf_log_printf (psf, "wavpack_write_strings: null pwvpk\n") ;
		psf->error = SFE_INTERNAL ;
		return 0 ;
		} ;

	int tag_count = WavpackGetNumTagItems (pwvpk->context) ;
	int i ;

	for (i = 0 ; i < tag_count ; ++ i)
	{	/* query key */
		int key_length = WavpackGetTagItemIndexed (pwvpk->context, i, NULL, 0) ;
		if (key_length < 4 || key_length > 11) // shortest is `date`, longest is `tracknumber`
		{	psf_log_printf (psf, "wavpack_read_strings: unknown key, length is %d\n", key_length) ;
			continue ;
			} ;
		char *key = calloc (key_length + 1, 1) ;
		if (key == NULL)
			return SFE_MALLOC_FAILED ;
		WavpackGetTagItemIndexed (pwvpk->context, i, key, key_length + 1) ;

		/* lowercase */
		int k ;
		for (k = 0 ; k < key_length ; ++ k)
		{	char codepoint = key [k] ;
			/* APEv2 tag only allows 0x20 to 0x7E in keys */
			if (codepoint >= 0x20 && codepoint <= 0x7E)
				key [k] = tolower (codepoint) ;
			else
			{	psf_log_printf (psf, "wavpack_read_strings: unknown key: `%s`\n", key) ;
				free (key) ;
				continue ;
				} ;
			} ;

		/* query libsndfile tag type from key */
		int sf_tag_type = 1 ;
		for (k = 0 ; k < (int) ARRAY_LEN (tag_key_table) ; ++ k)
		{	if (strcmp (tag_key_table [k].key, key) == 0)
			{	sf_tag_type = tag_key_table [k].sf_tag_type ;
				break ;
				} ;
			} ;
		if (sf_tag_type == 0)
		{	psf_log_printf (psf, "wavpack_read_strings: unknown key: `%s`\n", key) ;
			free (key) ;
			continue ;
			} ;

		/* query value */
		int val_length = WavpackGetTagItem (pwvpk->context, key, NULL, 0) ;
		if (val_length == 0)
		{	free (key) ;
			continue ;
			} ;
		char *val = calloc (val_length + 1, 1) ;
		if (val == NULL)
			return SFE_MALLOC_FAILED ;
		WavpackGetTagItem (pwvpk->context, key, val, val_length + 1) ;

		/* store and cleanup */
		int err = psf_store_string (psf, sf_tag_type, val) ;
		psf_log_printf (psf, "wavpack_read_strings: got string key=`%s`, val=`%s`\n", key, val) ;
		free (key) ;
		free (val) ;
		if (err != SFE_NO_ERROR)
		{	psf_log_printf (psf, "wavpack_read_strings: failed to call `psf_store_string`, err=%d.\n", err) ;
			return err ;
			} ;
		} ;

	return SFE_NO_ERROR ;
} /* wavpack_read_strings */

static int
wavpack_write_strings (SF_PRIVATE *psf)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	if (pwvpk == NULL)
	{	psf_log_printf (psf, "wavpack_write_strings: null pwvpk\n") ;
		psf->error = SFE_INTERNAL ;
		return 0 ;
		} ;

	int k, string_count = 0 ;

	for (k = 0 ; k < SF_MAX_STRINGS ; ++ k)
	{	if (psf->strings.data [k].type != 0)
			string_count ++ ;
		} ;

	if (string_count == 0)
		return SFE_NO_ERROR ;

	for (k = 0 ; k < SF_MAX_STRINGS && psf->strings.data [k].type != 0 ; ++ k)
	{	const char *key, *value ;

		switch (psf->strings.data [k].type)
		{	case SF_STR_SOFTWARE :
				key = "software" ;
				break ;
			case SF_STR_TITLE :
				key = "title" ;
				break ;
			case SF_STR_COPYRIGHT :
				key = "copyright" ;
				break ;
			case SF_STR_ARTIST :
				key = "artist" ;
				break ;
			case SF_STR_COMMENT :
				key = "comment" ;
				break ;
			case SF_STR_DATE :
				key = "year" ;
				break ;
			case SF_STR_ALBUM :
				key = "album" ;
				break ;
			case SF_STR_LICENSE :
				key = "license" ;
				break ;
			case SF_STR_TRACKNUMBER :
				key = "track" ;
				break ;
			case SF_STR_GENRE :
				key = "genre" ;
				break ;
			default :
				psf_log_printf (psf, "wavpack_write_strings: Unknown tag type `%d` @ string.data[%d]\n", psf->strings.data [k].type, k) ;
				continue ;
			} ;

		value = psf->strings.storage + psf->strings.data [k].offset ;
		psf_log_printf (psf, "wavpack_write_strings: key=`%s`, val=`%s`\n", key, value) ;

		if (WavpackAppendTagItem (pwvpk->context, key, value, strlen (value)) == 0)
		{	psf_log_printf (psf, "wavpack_write_strings: failed to call `WavpackAppendTagItem`, idx=%d, key=`%s`, value=`%s`\n", k, key, value) ;
			return SFE_STR_WEIRD ;
			} ;
		} ;

	if (WavpackWriteTag (pwvpk->context) == 0)
	{	psf_log_printf (psf, "wavpack_write_strings: failed to call `WavpackWriteTag`\n") ;
		return SFE_STR_WEIRD ;
		} ;

	return SFE_NO_ERROR ;
} /* wavpack_write_strings */

static int
wavpack_write_header (SF_PRIVATE *psf, int UNUSED (calc_length))
{	WAVPACK_PRIVATE* pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	int err ;

	if (psf->have_written)
		return SFE_CMD_HAS_DATA ;
	if (pwvpk->is_header_written)
	{	if (pwvpk->is_random_access_device)
		{	if (pwvpk->context && (err = wavpack_enc_init (psf)) != SFE_NO_ERROR)
			{	psf_log_printf (psf, "wavpack_write_header: failed to reinitialize encoder\n") ;
				return err ;
				} ;
			pwvpk->is_header_written = 0 ;
			}
		else
		{	psf_log_printf (psf, "wavpack_write_header: unable to reinitialize encoder on sequential device.\n") ;
			return SFE_NOT_SEEKABLE ;
			} ;
		} ;

	int64_t total_samples = psf->sf.frames <= 0 ? -1 : psf->sf.frames ;
	if (WavpackSetConfiguration64 (pwvpk->context, &pwvpk->config, total_samples, NULL) == 0)
	{	psf_log_printf (psf, "wavpack_write_header: failed to call `WavpackSetConfiguration64`\n") ;
		return SFE_UNSUPPORTED_ENCODING ;
		} ;

	if (WavpackPackInit (pwvpk->context) == 0)
	{	psf_log_printf (psf, "wavpack_write_header: failed to call `WavpackPackInit`\n") ;
		return SFE_UNSUPPORTED_ENCODING ;
		} ;

	pwvpk->is_header_written = 1 ;

	return SFE_NO_ERROR ;
} /* wavpack_write_header */

static void *
wavpack_ensure_buffer (WAVPACK_PRIVATE *pwvpk)
{	if (pwvpk->buffer == NULL)
		pwvpk->buffer = calloc (WAVPACK_BUFFER_LENGTH / 8, 8) ;
	return pwvpk->buffer ;
} /* wavpack_ensure_buffer */

static sf_count_t
wavpack_read_float (SF_PRIVATE *psf, float *ptr, sf_count_t len)
{	return wavpack_unpack_buffer_any (psf, ptr, len, SF_FORMAT_FLOAT) ;
} /* wavpack_read_float */

static sf_count_t
wavpack_read_double (SF_PRIVATE *psf, double *ptr, sf_count_t len)
{	return wavpack_unpack_buffer_any (psf, ptr, len, SF_FORMAT_DOUBLE) ;
} /* wavpack_read_double */

static sf_count_t
wavpack_read_int (SF_PRIVATE *psf, int *ptr, sf_count_t len)
{	return wavpack_unpack_buffer_any (psf, ptr, len, SF_FORMAT_PCM_32) ;
} /* wavpack_read_double */

static sf_count_t
wavpack_read_short (SF_PRIVATE *psf, short *ptr, sf_count_t len)
{	return wavpack_unpack_buffer_any (psf, ptr, len, SF_FORMAT_PCM_16) ;
} /* wavpack_read_double */

static inline int
wavpack_pack_buffer_single_any
(	SF_PRIVATE *psf, sf_count_t *total_packed_count, const void *ptr, sf_count_t i_chunk, sf_count_t chunk_item_count, sf_count_t item_count, int32_t *buffer,
	int itype, int otype, float vlim, int shl, int voffset
)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	const short *in_ptr_short = (((const short *) ptr) + i_chunk * chunk_item_count) ;
	const int *in_ptr_int = (((const int *) ptr) + i_chunk * chunk_item_count) ;
	const float *in_ptr_float = (((const float *) ptr) + i_chunk * chunk_item_count) ;
	const double *in_ptr_double = (((const double *) ptr) + i_chunk * chunk_item_count) ;

	if (otype == SF_FORMAT_FLOAT)
	{	if (itype == SF_FORMAT_FLOAT)
			memcpy (buffer, in_ptr_float, item_count * sizeof (float)) ;
		else if (itype == SF_FORMAT_DOUBLE)
			wavpack_cvtf64f32 (in_ptr_double, ((float *) buffer), item_count) ;
		else if (itype == SF_FORMAT_PCM_16)
			wavpack_cvti16f32 (in_ptr_short, ((float *) buffer), item_count, 1) ;
		else if (itype == SF_FORMAT_PCM_32)
			wavpack_cvti32f32 (in_ptr_int, ((float *) buffer), item_count, 1) ;
		else
			goto on_internal_error ;
		}
	else if (itype == SF_FORMAT_FLOAT)
		// wavpack_cvtf*i* (const FTYPE *src, ITYPE *dest, sf_count_t count, int normalize, FTYPE vlim, ITYPE offset, int use_clip)
		wavpack_cvtf32i32 (in_ptr_float, buffer, item_count, psf->norm_float, vlim, voffset, psf->add_clipping) ;
	else if (itype == SF_FORMAT_DOUBLE)
		wavpack_cvtf64i32 (in_ptr_double, buffer, item_count, psf->norm_double, vlim, voffset, psf->add_clipping) ;
	else if (itype == SF_FORMAT_PCM_16)
		wavpack_shifti16i32 (in_ptr_short, buffer, item_count, shl, voffset) ;
	else if (itype == SF_FORMAT_PCM_32)
		wavpack_shifti32i32 (in_ptr_int, buffer, item_count, shl, voffset) ;
	else
		goto on_internal_error ;

	if (WavpackPackSamples (pwvpk->context, buffer, item_count / psf->sf.channels) == 0)
	{	psf_log_printf (psf, "wavpack_pack_buffer_single_any: failed to pack %ld samples`.\n", item_count) ;
		return SFE_INTERNAL ;
		} ;

	*total_packed_count += item_count ;
	return SFE_NO_ERROR ;

on_internal_error:
	psf_log_printf (psf, "wavpack_pack_buffer_single_any: invalid (itype, otype) = (%d, %d).\n", itype, otype) ;
	return SFE_INTERNAL ;
}

static inline int
wavpack_unpack_buffer_single_any
(	SF_PRIVATE *psf, sf_count_t *total_packed_count, void *ptr, sf_count_t i_chunk, sf_count_t chunk_item_count, sf_count_t item_count, int32_t *buffer,
	int itype, int otype, float vlim, int shl, int voffset
)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	short *out_ptr_short = (((short *) ptr) + i_chunk * chunk_item_count) ;
	int *out_ptr_int = (((int *) ptr) + i_chunk * chunk_item_count) ;
	float *out_ptr_float = (((float *) ptr) + i_chunk * chunk_item_count) ;
	double *out_ptr_double = (((double *) ptr) + i_chunk * chunk_item_count) ;

	if (WavpackUnpackSamples (pwvpk->context, buffer, item_count / psf->sf.channels) == 0)
	{	psf_log_printf (psf, "wavpack_unpack_buffer_single_any: failed to unpack %ld samples`.\n", item_count) ;
		return SFE_MALFORMED_FILE ;
		} ;

	if (otype == SF_FORMAT_DOUBLE)
	{	if (itype == SF_FORMAT_FLOAT)
			wavpack_cvtf32f64 (((const float *) buffer), out_ptr_double, item_count) ;
		else if (itype == SF_FORMAT_PCM_U8 || itype == SF_FORMAT_PCM_S8)
			wavpack_cvt32i8f64 (buffer, out_ptr_double, item_count, psf->norm_double) ;
		else if (itype == SF_FORMAT_PCM_16)
			wavpack_cvt32i16f64 (buffer, out_ptr_double, item_count, psf->norm_double) ;
		else if (itype == SF_FORMAT_PCM_24)
			wavpack_cvt32i24f64 (buffer, out_ptr_double, item_count, psf->norm_double) ;
		else if (itype == SF_FORMAT_PCM_32)
			wavpack_cvti32f64 (buffer, out_ptr_double, item_count, psf->norm_double) ;
		else
			goto on_internal_error ;
		}
	else if (otype == SF_FORMAT_FLOAT)
	{	if (itype == SF_FORMAT_FLOAT)
			memcpy (out_ptr_float, buffer, item_count * sizeof (float)) ;
		else if (itype == SF_FORMAT_PCM_U8 || itype == SF_FORMAT_PCM_S8)
			wavpack_cvt32i8f32 (buffer, out_ptr_float, item_count, psf->norm_float) ;
		else if (itype == SF_FORMAT_PCM_16)
			wavpack_cvt32i16f32 (buffer, out_ptr_float, item_count, psf->norm_float) ;
		else if (itype == SF_FORMAT_PCM_24)
			wavpack_cvt32i24f32 (buffer, out_ptr_float, item_count, psf->norm_float) ;
		else if (itype == SF_FORMAT_PCM_32)
			wavpack_cvti32f32 (buffer, out_ptr_float, item_count, psf->norm_float) ;
		else
			goto on_internal_error ;
		}
	else if (itype == SF_FORMAT_FLOAT)
	{	if (otype == SF_FORMAT_PCM_32)
			wavpack_cvtf32i32 ((const float *) buffer, out_ptr_int, item_count, 1, vlim, voffset, psf->add_clipping) ;
		else if (otype == SF_FORMAT_PCM_16)
			wavpack_cvtf32i16 ((const float *) buffer, out_ptr_short, item_count, 1, vlim, voffset, psf->add_clipping) ;
		else
			goto on_internal_error ;
		}
	else if (itype == SF_FORMAT_PCM_16 || itype == SF_FORMAT_PCM_24 || itype == SF_FORMAT_PCM_32 || itype == SF_FORMAT_PCM_U8 || itype == SF_FORMAT_PCM_S8)
	{	if (otype == SF_FORMAT_PCM_32)
			wavpack_shifti32i32 (buffer, out_ptr_int, item_count, shl, voffset) ;
		else if (otype == SF_FORMAT_PCM_16)
			wavpack_shifti32i16 (buffer, out_ptr_short, item_count, shl, voffset) ;
		else
			goto on_internal_error ;
		}
	else
		goto on_internal_error ;

	*total_packed_count += item_count ;
	return SFE_NO_ERROR ;

on_internal_error:
	psf_log_printf (psf, "wavpack_unpack_buffer_single_any: invalid (itype, otype) = (%d, %d).\n", itype, otype) ;
	return SFE_INTERNAL ;
}

static sf_count_t
wavpack_unpack_buffer_any (SF_PRIVATE *psf, void *ptr, sf_count_t sample_count, int otype)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	int err ;

	if (pwvpk == NULL)
	{	psf_log_printf (psf, "wavpack_unpack_buffer_any: null pwvpk\n") ;
		psf->error = SFE_INTERNAL ;
		return 0 ;
		} ;

	if (pwvpk->context == NULL)
	{	psf_log_printf (psf, "wavpack_unpack_buffer_any: null pwvpk->context\n") ;
		psf->error = SFE_INTERNAL ;
		return 0 ;
		} ;

	if (pwvpk->is_seek_end)
	{	psf_log_printf (psf, "wavpack_unpack_buffer_any: reach seek end\n") ;
		return 0 ;
		} ;
	if (sample_count == 0)
		return 0 ;

	int itype = SF_CODEC (psf->sf.format) ;

	/* fast path */
	if (sample_count <= WVPK_UNPACK_FAST_PATH_MAX_SAMPLES && (itype == SF_FORMAT_PCM_32 || itype == SF_FORMAT_FLOAT) && (otype == SF_FORMAT_PCM_32 || otype == SF_FORMAT_FLOAT))
	{	if (WavpackUnpackSamples (pwvpk->context, ptr, sample_count / psf->sf.channels) == 0)
		{	psf_log_printf (psf, "wavpack_unpack_buffer_any: failed to unpack %ld samples\n", sample_count) ;
			psf->error = SFE_MALFORMED_FILE ;
			return 0 ;
		}
		if (itype != otype)
		{	if (itype == SF_FORMAT_PCM_32)
				wavpack_cvti32f32_inplace (ptr, sample_count, psf->norm_float) ;
			else if (itype == SF_FORMAT_FLOAT)
				wavpack_cvtf32i32_inplace (ptr, sample_count, psf->norm_float, 2147483648.0, 0, psf->add_clipping) ;
			else
				goto on_internal_error ;
			} ;
		return sample_count ;
		} ;

	/* general path */
	int32_t *buffer = wavpack_ensure_buffer (pwvpk) ;
	if (buffer == NULL)
	{	psf->error = SFE_MALLOC_FAILED ;
		return 0 ;
		} ;

	sf_count_t buffer_sample_count = WAVPACK_BUFFER_LENGTH / sizeof (int32_t) / psf->sf.channels ;
	sf_count_t full_chunk_count = (sample_count / psf->sf.channels) / buffer_sample_count ;
	sf_count_t chunk_item_count = buffer_sample_count * psf->sf.channels ;
	sf_count_t remain_item_count = ((sample_count / psf->sf.channels) % buffer_sample_count) * psf->sf.channels ;
	float vlim = 0.0 ;
	int shl = 0 ;
	int voffset = 0 ;

	switch (otype)
	{	case SF_FORMAT_FLOAT:
		case SF_FORMAT_DOUBLE:
			break ;
		case SF_FORMAT_PCM_16:
			switch (itype)
			{	case SF_FORMAT_PCM_U8 :
				case SF_FORMAT_PCM_S8 :
					shl = 8 ;
					break ;
				case SF_FORMAT_PCM_16 :
					break ;
				case SF_FORMAT_PCM_24 :
					shl = -8 ;
					break ;
				case SF_FORMAT_PCM_32 :
					shl = -16 ;
					break ;
				case SF_FORMAT_FLOAT :
					vlim = (float) WVPK_INT16_VLIM ;
					break ;
				default :
					goto on_internal_error ;
				} ;
			break ;
		case SF_FORMAT_PCM_32:
			switch (itype)
			{	case SF_FORMAT_PCM_U8 :
				case SF_FORMAT_PCM_S8 :
					shl = 24 ;
					break ;
				case SF_FORMAT_PCM_16 :
					shl = 16 ;
					break ;
				case SF_FORMAT_PCM_24 :
					shl = 8 ;
					break ;
				case SF_FORMAT_PCM_32 :
					break ;
				case SF_FORMAT_FLOAT :
					vlim = (float) WVPK_INT32_VLIM ;
					break ;
				default :
					goto on_internal_error ;
				} ;
			break ;
		default:
			goto on_internal_error ;
		} ;


	sf_count_t total_unpacked_count = 0 ;
	/* full chunks */
	for (sf_count_t i_chunk = 0 ; i_chunk < full_chunk_count ; ++ i_chunk)
	{	err = wavpack_unpack_buffer_single_any
		(	psf, &total_unpacked_count, ptr, i_chunk, chunk_item_count, chunk_item_count, buffer,
			itype, otype, vlim, shl, voffset
			) ;
		if (err != SFE_NO_ERROR)
		{	psf->error = err ;
			return total_unpacked_count ;
			} ;
		} ;

	/* remain */
	err = wavpack_unpack_buffer_single_any
	(	psf, &total_unpacked_count, ptr, full_chunk_count, chunk_item_count, remain_item_count, buffer,
		itype, otype, vlim, shl, voffset
		) ;

	if (err != SFE_NO_ERROR)
		psf->error = err ;

	return total_unpacked_count ;

on_internal_error:
	psf_log_printf (psf, "wavpack_unpack_buffer_any: invalid (itype, otype, sample_count) = (%d, %d, %ld)\n", itype, otype, sample_count) ;
	psf->error = SFE_INTERNAL ;
	return 0 ;
}

static sf_count_t
wavpack_pack_buffer_any (SF_PRIVATE *psf, const void *ptr, sf_count_t sample_count, int itype)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	int err ;

	if (pwvpk == NULL)
	{	psf_log_printf (psf, "wavpack_pack_buffer_any: null pwvpk\n") ;
		psf->error = SFE_INTERNAL ;
		return 0 ;
		} ;

	if (pwvpk->context == NULL)
	{	psf_log_printf (psf, "wavpack_pack_buffer_any: null pwvpk->context\n") ;
		psf->error = SFE_INTERNAL ;
		return 0 ;
		} ;

	if (pwvpk->is_seek_end)
	{	psf_log_printf (psf, "wavpack_pack_buffer_any: seek end flag should not exists here\n") ;
		psf->error = SFE_INTERNAL ;
		return 0 ;
		} ;

	if (sample_count == 0)
		return 0 ;

	int32_t *buffer = wavpack_ensure_buffer (pwvpk) ;
	if (buffer == NULL)
	{	psf->error = SFE_MALLOC_FAILED ;
		return 0 ;
		} ;

	sf_count_t buffer_sample_count = WAVPACK_BUFFER_LENGTH / sizeof (int32_t) / psf->sf.channels ;
	sf_count_t full_chunk_count = (sample_count / psf->sf.channels) / buffer_sample_count ;
	sf_count_t chunk_item_count = buffer_sample_count * psf->sf.channels ;
	sf_count_t remain_item_count = ((sample_count / psf->sf.channels) % buffer_sample_count) * psf->sf.channels ;

	float vlim = 0.0 ;
	int shl = 0 ;
	int voffset = 0 ;

	int otype = SF_CODEC (psf->sf.format) ;

	switch (itype)
	{	case SF_FORMAT_FLOAT:
		case SF_FORMAT_DOUBLE:
			switch (otype)
			{	case SF_FORMAT_PCM_U8 :
				case SF_FORMAT_PCM_S8 :
					vlim = (float) WVPK_INT8_VLIM ;
					break ;
				case SF_FORMAT_PCM_16 :
					vlim = (float) WVPK_INT16_VLIM ;
					break ;
				case SF_FORMAT_PCM_24 :
					vlim = (float) WVPK_INT24_VLIM ;
					break ;
				case SF_FORMAT_PCM_32 :
					vlim = (float) WVPK_INT32_VLIM ;
					break ;
				case SF_FORMAT_FLOAT :
					break ;
				default :
					goto on_internal_error ;
				} ;
			break ;
		case SF_FORMAT_PCM_16:
			switch (otype)
			{	case SF_FORMAT_PCM_U8 :
				case SF_FORMAT_PCM_S8 :
					shl = -8 ;
					break ;
				case SF_FORMAT_PCM_16 :
					break ;
				case SF_FORMAT_PCM_24 :
					shl = 8 ;
					break ;
				case SF_FORMAT_PCM_32 :
					shl = 16 ;
					break ;
				case SF_FORMAT_FLOAT :
					break ;
				default :
					goto on_internal_error ;
				} ;
			break ;
		case SF_FORMAT_PCM_32:
			switch (otype)
			{	case SF_FORMAT_PCM_U8 :
				case SF_FORMAT_PCM_S8 :
					shl = -24 ;
					break ;
				case SF_FORMAT_PCM_16 :
					shl = -16 ;
					break ;
				case SF_FORMAT_PCM_24 :
					shl = -8 ;
					break ;
				case SF_FORMAT_PCM_32 :
					break ;
				case SF_FORMAT_FLOAT :
					break ;
				default :
					goto on_internal_error ;
				} ;
			break ;
		default:
			goto on_internal_error ;
		} ;

	sf_count_t total_packed_count = 0 ;
	/* full chunks */
	for (sf_count_t i_chunk = 0 ; i_chunk < full_chunk_count ; ++ i_chunk)
	{	err = wavpack_pack_buffer_single_any
		(	psf, &total_packed_count, ptr, i_chunk, chunk_item_count, chunk_item_count, buffer,
			itype, otype, vlim, shl, voffset
			) ;
		if (err != SFE_NO_ERROR)
		{	psf->error = err ;
			return total_packed_count ;
			} ;
		} ;

	/* remain */
	err = wavpack_pack_buffer_single_any
	(	psf, &total_packed_count, ptr, full_chunk_count, chunk_item_count, remain_item_count, buffer,
		itype, otype, vlim, shl, voffset
		) ;

	if (err != SFE_NO_ERROR)
		psf->error = err ;

	return total_packed_count ;

on_internal_error:
	psf_log_printf (psf, "wavpack_pack_buffer_any: invalid (itype, otype, sample_count) = (%d, %d, %ld)\n", itype, otype, sample_count) ;
	psf->error = SFE_INTERNAL ;
	return 0 ;
}

static sf_count_t
wavpack_write_float (SF_PRIVATE *psf, const float *ptr, sf_count_t sample_count)
{	return wavpack_pack_buffer_any (psf, ptr, sample_count, SF_FORMAT_FLOAT) ;
} /* wavpack_write_float */

static sf_count_t
wavpack_write_double (SF_PRIVATE *psf, const double *ptr, sf_count_t sample_count)
{	return wavpack_pack_buffer_any (psf, ptr, sample_count, SF_FORMAT_DOUBLE) ;
} /* wavpack_write_double */

static sf_count_t
wavpack_write_short (SF_PRIVATE *psf, const short *ptr, sf_count_t sample_count)
{	return wavpack_pack_buffer_any (psf, ptr, sample_count, SF_FORMAT_PCM_16) ;
} /* wavpack_write_short */

static sf_count_t
wavpack_write_int (SF_PRIVATE *psf, const int *ptr, sf_count_t sample_count)
{	return wavpack_pack_buffer_any (psf, ptr, sample_count, SF_FORMAT_PCM_32) ;
} /* wavpack_write_int */

static int
wavpack_close (SF_PRIVATE *psf)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	if (pwvpk == NULL || pwvpk->is_in_close)
		return SFE_NO_ERROR ;

	int err = SFE_NO_ERROR ;
	pwvpk->is_in_close = 1 ;

	if (pwvpk->context)
	{	if (psf->file.mode == SFM_WRITE)
		{	err = WavpackFlushSamples (pwvpk->context) != 0 ? SFE_NO_ERROR : SFE_INTERNAL ;
			if (err != SFE_NO_ERROR)
			{	psf_log_printf (psf, "wavpack_close: failed to flush samples\n") ;
				goto context_clean_up ;
				} ;
			if (pwvpk->first_block)
			{	/* update header data in first block, write it back */
				WavpackUpdateNumSamples (pwvpk->context, pwvpk->first_block) ;
				int64_t orig_pos = sf_wavpack_get_pos_callback (psf) ;
				if (orig_pos == -1)
				{	err = psf->error ;
					psf_log_printf (psf, "wavpack_close: failed to compute `orig_pos`\n") ;
					goto context_clean_up ;
					} ;
				if (sf_wavpack_set_pos_abs_callback (psf, pwvpk->first_pos) != 0)
				{	err = psf->error ;
					psf_log_printf (psf, "wavpack_close: failed to seek to `first_pos`\n") ;
					goto context_clean_up ;
					} ;
				if (sf_wavpack_write_bytes_callback (psf, pwvpk->first_block, pwvpk->first_bcount) != pwvpk->first_bcount)
				{	err = psf->error ;
					psf_log_printf (psf, "wavpack_close: failed to write `first_block`\n") ;
					goto context_clean_up ;
					} ;
				if (sf_wavpack_set_pos_abs_callback (psf, orig_pos) != 0)
				{	err = psf->error ;
					psf_log_printf (psf, "wavpack_close: failed to seek to `orig_pos`\n") ;
					goto context_clean_up ;
					} ;
				} ;
			err = wavpack_write_strings (psf) ;
			if (err != SFE_NO_ERROR)
			{	psf_log_printf (psf, "wavpack_close: failed to call `wavpack_write_strings`\n") ;
				goto context_clean_up ;
				} ;
			} ;

context_clean_up:
		pwvpk->context = WavpackCloseFile (pwvpk->context) ;
		} ;

	if (pwvpk->buffer)
		free (pwvpk->buffer) ;

	if (pwvpk->first_block)
		free (pwvpk->first_block) ;

	free (pwvpk) ;
	psf->codec_data = NULL ;

	return err ;
} /* wavpack_close */

static sf_count_t
wavpack_seek (SF_PRIVATE *psf, int UNUSED (mode), sf_count_t offset)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;

	if (!psf->sf.seekable)
	{	psf_log_printf (psf, "wavpack_seek: not seekable for file mode %d, is_random_access_device=%d\n", psf->file.mode, pwvpk->is_random_access_device) ;
		psf->error = SFE_NOT_SEEKABLE ;
		return -1 ;
		} ;

	if (pwvpk == NULL)
	{	psf_log_printf (psf, "wavpack_seek: null pwvpk\n") ;
		psf->error = SFE_INTERNAL ;
		return -1 ;
		} ;

	if (pwvpk->context == NULL)
	{	psf_log_printf (psf, "wavpack_seek: null pwvpk->context\n") ;
		psf->error = SFE_INTERNAL ;
		return -1 ;
		} ;

	if (psf->dataoffset < 0)
	{	psf->error = SFE_BAD_SEEK ;
		return -1 ;
		} ;

	if (offset == psf->sf.frames)
	{	pwvpk->is_seek_end = 1 ;
		return offset ;
		} ;

	pwvpk->is_seek_end = 0 ;
	if (WavpackSeekSample64 (pwvpk->context, offset) != 0)
		return offset ;
	else
	{	psf->error = SFE_SEEK_FAILED ;
		wavpack_close (psf) ;
		return -1 ;
		} ;
} /* wavpack_seek */

static int
wavpack_byterate (SF_PRIVATE *psf)
{	if (psf->file.mode == SFM_READ)
		return (psf->datalength * psf->sf.samplerate) / psf->sf.frames ;
	return -1 ;
} /* wavpack_byterate */

static void
wavpack_cvtf64f32 (const double *src, float *dest, sf_count_t count)
{	sf_count_t i ;
	for (i = 0 ; i < count ; ++ i)
		dest [i] = (float) src [i] ;
} /* wavpack_cvtf64f32 */

static void
wavpack_cvtf32f64 (const float *src, double *dest, sf_count_t count)
{	sf_count_t i ;
	for (i = 0 ; i < count ; ++ i)
		dest [i] = (double) src [i] ;
} /* wavpack_cvtf32f64 */

static int32_t
sf_wavpack_read_bytes_callback (void *client_data, void *data, int32_t bcount)
{	static const unsigned char magic_header [4] = { 'w', 'v', 'p', 'k' } ;

	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	int32_t read_bcount = 0 ;

	if (bcount <= 0)
	{	psf_log_printf (psf, "sf_wavpack_read_bytes_callback: unable to read %d bytes.\n", bcount) ;
		return 0 ;
		} ;

	if (!pwvpk->is_random_access_device)
	{	/* simulate ungetc(), see `sf_wavpack_push_back_byte_callback()` */
		if (read_bcount != bcount && pwvpk->last_char_status == LAST_CHAR_VALID)
		{	((unsigned char*) data) [read_bcount] = pwvpk->last_char ;
			pwvpk->last_char_status = LAST_CHAR_INVALID ;
			read_bcount += 1 ;
			pwvpk->sequential_pos += 1 ;
			} ;

		/* magic bytes in header was taken by `guess_file_type()`, restore it here */
		while (read_bcount != bcount && pwvpk->sequential_pos < 4)
		{	((unsigned char*) data) [read_bcount] = magic_header [pwvpk->sequential_pos] ;
			pwvpk->sequential_pos += 1 ;
			read_bcount += 1 ;
			} ;
		} ;

	/* continue read data */
	sf_count_t bytes ;
	if (read_bcount != bcount)
	{	bytes = psf_fread (((unsigned char*) data) + read_bcount, 1, bcount - read_bcount, psf) ;
		if (bytes > 0)
		{	read_bcount += bytes ;
			pwvpk->sequential_pos += bytes ;
			} ;
		} ;

	/*
	fprintf (stderr, "devtype=%d read %d bytes: `", pwvpk->is_random_access_device, bcount) ;
	for (int i = 0; i < read_bcount; ++i)
	{	unsigned char v = ((unsigned char*)data)[i] ;
		fprintf (stderr, "%02x", (int)v) ;
		if (v < 0x20 || v > 0x7e)
			fprintf (stderr, ":? ") ;
		else
			fprintf (stderr, ":%c ", v) ;
		} ;
	fprintf (stderr, "`\n") ;
	*/

	if (psf->error == SFE_NO_ERROR)
		return read_bcount ;
	else
	{	psf_log_printf (psf, "sf_wavpack_read_bytes_callback: `psf_fread` failed, error is `%s`(%d)\n", sf_strerror ((SNDFILE*) psf), psf->error) ;
		return 0 ;
		} ;
} /* sf_wavpack_read_bytes_callback */

static int32_t
sf_wavpack_write_bytes_callback (void *client_data, void *data, int32_t bcount)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;

	if (pwvpk->is_random_access_device && !pwvpk->first_block)
	{	/* record the first block written to update sample count in header when closing a file */
		pwvpk->first_bcount = bcount ;
		pwvpk->first_pos = sf_wavpack_get_pos_callback (client_data) ;
		if (pwvpk->first_pos == -1)
		{	psf_log_printf (psf, "sf_wavpack_write_bytes_callback: failed to compute `first_pos`\n") ;
			return 0 ;
			} ;
		pwvpk->first_block = calloc ((bcount + 15) / 16, 16) ;
		if (!pwvpk->first_block)
		{	psf->error = SFE_MALLOC_FAILED ;
			return 0 ;
		}
		memcpy (pwvpk->first_block, data, bcount) ;
		} ;

	sf_count_t bytes = psf_fwrite (data, 1, bcount, psf) ;

	if (psf->error == SFE_NO_ERROR)
		return bytes ;
	else
		return 0 ;
} /* sf_wavpack_write_bytes_callback */

static int64_t
sf_wavpack_get_pos_callback (void *client_data)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	sf_count_t absolute_byte_offset ;

	if (!pwvpk->is_random_access_device)
	{	psf_log_printf (psf, "sf_wavpack_get_pos_callback: unable to do this on a sequential device\n") ;
		return -1 ;
		} ;

	absolute_byte_offset = psf_ftell (psf) ;
	if (psf->error == SFE_NO_ERROR)
		return absolute_byte_offset ;
	else
	{	psf_log_printf (psf, "sf_wavpack_get_pos_callback: `psf_ftell` failed, error is `%s`(%d)\n", sf_strerror ((SNDFILE*) psf), psf->error) ;
		return -1 ;
		} ;
} /* sf_wavpack_get_pos_callback */

static int
sf_wavpack_set_pos_abs_callback (void *client_data, int64_t pos)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;

	if (!pwvpk->is_random_access_device)
	{	psf_log_printf (psf, "sf_wavpack_set_pos_abs_callback: unable to do this on a sequential device, pos=%ld\n", pos) ;
		return -1 ;
		} ;

	psf_fseek (psf, pos, SEEK_SET) ;
	if (psf->error == SFE_NO_ERROR)
		return 0 ;
	else
	{	psf_log_printf (psf, "sf_wavpack_set_pos_abs_callback: `psf_fseek` failed, error is `%s`(%d)\n", sf_strerror ((SNDFILE*) psf), psf->error) ;
		return -1 ;
		} ;
} /* sf_wavpack_set_pos_abs_callback */

static int
sf_wavpack_set_pos_rel_callback (void *client_data, int64_t delta, int mode)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;

	if (!pwvpk->is_random_access_device)
	{	psf_log_printf (psf, "sf_wavpack_set_pos_rel_callback: unable to do this on a sequential device\n") ;
		return -1 ;
		} ;

	psf_fseek (psf, delta, mode) ;
	if (psf->error == SFE_NO_ERROR)
		return 0 ;
	else
	{	psf_log_printf (psf, "sf_wavpack_set_pos_rel_callback: `psf_fseek` failed, error is `%s`(%d)\n", sf_strerror ((SNDFILE*) psf), psf->error) ;
		return -1 ;
		} ;
} /* sf_wavpack_set_pos_rel_callback */

static int
sf_wavpack_push_back_byte_callback (void *client_data, int c)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;

	if (!pwvpk->is_random_access_device)
	{	/* simulate ungetc(), see `sf_wavpack_read_bytes_callback()` */
		if (pwvpk->sequential_pos > 0)
		{	pwvpk->last_char = c ;
			pwvpk->last_char_status = LAST_CHAR_VALID ;
			pwvpk->sequential_pos -= 1 ;
			return c ;
			}
		else
			return -1 ;
		}
	else
	{	// we need not check `c` because wavpack does not modify it
		psf_fseek (psf, -1, SEEK_CUR) ;
		if (psf->error == SFE_NO_ERROR)
			return c ;
		else
			return -1 ;
		} ;
} /* sf_wavpack_push_back_byte_callback */

static int64_t
sf_wavpack_get_length_callback (void *client_data)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;

	if (!pwvpk->is_random_access_device)
	{	psf_log_printf (psf, "sf_wavpack_get_length_callback: unable to do this on a sequential device\n") ;
		return 0 ;
		} ;

	sf_count_t orig_abs_offset, file_len ;

	orig_abs_offset = psf_ftell (psf) ;
	if (psf->error != SFE_NO_ERROR)
		goto on_error ;

	file_len = psf_fseek (psf, 0, SEEK_END) ;
	if (psf->error != SFE_NO_ERROR)
		goto on_error ;

	psf_fseek (psf, orig_abs_offset, SEEK_SET) ;
	if (psf->error != SFE_NO_ERROR)
		goto on_error ;
	return file_len ;

on_error :
	psf_log_printf (psf, "sf_wavpack_get_length_callback: failed, error is `%s`(%d)\n", sf_strerror ((SNDFILE*) psf), psf->error) ;
	return 0 ;
} /* sf_wavpack_get_length_callback */


static int
sf_wavpack_can_seek_callback (void *client_data)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;

	return pwvpk->is_random_access_device ;
} /* sf_wavpack_can_seek_callback */

static int
sf_wavpack_truncate_here (void *client_data)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;

	if (!pwvpk->is_random_access_device)
	{	psf_log_printf (psf, "sf_wavpack_truncate_here: unable to do this on a sequential device\n") ;
		return 0 ;
		} ;

	int64_t pos = sf_wavpack_get_pos_callback (client_data) ;
	if (pos < 0)
	{	psf_log_printf (psf, "sf_wavpack_truncate_here: failed to compute current file position.\n") ;
		return -1 ;
		} ;
	if (psf_ftruncate (psf, pos) != 0)
	{	psf_log_printf (psf, "sf_wavpack_truncate_here: failed to call `psf_ftruncate`, current position is %lld.\n", pos) ;
		return -1 ;
		} ;

	return 0 ;
} /* sf_wavpack_truncate_here */

static int
wavpack_io_feature_test (SF_PRIVATE *psf)
{	int is_pipe ;
	sf_count_t orig_pos ;

	if ((is_pipe = psf_is_pipe (psf)) != 0)
		return 0 ;

	orig_pos = psf_ftell (psf) ;
	if (psf->error != SFE_NO_ERROR)
	{	psf_log_printf (psf, "wavpack_io_feature_test: failed to call `psf_ftell`, error is `%s`(%d)\n", sf_strerror ((SNDFILE*) psf), psf->error) ;
		goto on_sequential_device ;
		} ;

	psf_fseek (psf, orig_pos, SEEK_SET) ;
	if (psf->error != SFE_NO_ERROR)
	{	psf_log_printf (psf, "wavpack_io_feature_test: failed to call `psf_fseek (psf, orig_pos, SEEK_SET)`, error is `%s`(%d)\n", sf_strerror ((SNDFILE*) psf), psf->error) ;
		goto on_sequential_device ;
		} ;

	psf_log_printf (psf, "wavpack_io_feature_test: device feature level is `random access device`\n") ;
	return 1 ; // random_access_device

on_sequential_device:
	psf_log_printf (psf, "wavpack_io_feature_test: device feature level is `sequential device`\n") ;
	psf->error = SFE_NO_ERROR ;
	return 0 ;
} /* wavpack_io_feature_test */

/*------------------------------------------------------------------------------
*/

#else /* HAVE_WAVPACK */

int
wavpack_open	(SF_PRIVATE *psf)
{	psf_log_printf (psf, "This version of libsndfile was compiled without Wavpack support.\n") ;
	return SFE_UNIMPLEMENTED ;
} /* wavpack_open */

#endif /* HAVE_WAVPACK */
