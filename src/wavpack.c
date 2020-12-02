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

#if SIZEOF_SF_COUNT_T != 8
	#error Unsupported SIZEOF_SF_COUNT_T for WavPack wrapper.
#endif

#include	<wavpack/wavpack.h>

/*------------------------------------------------------------------------------
** Macros
*/

#define WAVPACK_GENERATE_FLOAT_TO_INT_FUNC(FUNC_NAME, ITYPE, FTYPE, LRFUNC, MAXFUNC, MINFUNC) \
static void \
FUNC_NAME (const FTYPE *src, ITYPE *dest, sf_count_t count, int normalize, FTYPE vlim, ITYPE offset, int use_clip) \
{	FTYPE normfact = normalize ? (vlim) : 1.0 ; \
		\
	if ((use_clip)) \
	{	for (int i = 0 ; i < count ; ++ i) \
			dest [i] = LRFUNC (MAXFUNC (- (vlim), MINFUNC (src [i] * normfact, ((vlim) - 1.0)))) + (offset) ; \
		} \
	else \
	{	for (int i = 0 ; i < count ; ++ i) \
			dest [i] = LRFUNC (src [i] * normfact) + (offset) ; \
		} ; \
} /* FUNC_NAME */

#define WAVPACK_GENERATE_FLOAT_TO_INT_FUNC_INPLACE(FUNC_NAME, ITYPE, FTYPE, LRFUNC, MAXFUNC, MINFUNC) \
static void \
FUNC_NAME (void *src_dest, sf_count_t count, int normalize, FTYPE vlim, ITYPE offset, int use_clip) \
{	FTYPE normfact = normalize ? (vlim) : 1.0 ; \
		\
	if ((use_clip)) \
	{	for (int i = 0 ; i < count ; ++ i) \
			* (((ITYPE *) src_dest) + i) = LRFUNC (MAXFUNC (- (vlim), MINFUNC ((* (((const FTYPE *) src_dest) + i)) * normfact, ((vlim) - 1.0)))) + (offset) ; \
		} \
	else \
	{	for (int i = 0 ; i < count ; ++ i) \
			* (((ITYPE *) src_dest) + i) = LRFUNC ((* (((const FTYPE *) src_dest) + i)) * normfact) + (offset) ; \
		} ; \
} /* FUNC_NAME */

#define WAVPACK_GENERATE_INT_TO_FLOAT_FUNC(FUNC_NAME, ITYPE, FTYPE, VLIM, OFFSET) \
static void \
FUNC_NAME (const ITYPE *src, FTYPE *dest, int count, int normalize) \
{	FTYPE normfact = normalize ? (VLIM) : 1.0 ; \
		\
	for (int i = 0 ; i < count ; ++ i) \
		dest [i] = (((FTYPE) src [i]) - (OFFSET)) / normfact ; \
} /* FUNC_NAME */

#define WAVPACK_GENERATE_INT_TO_FLOAT_FUNC_INPLACE(FUNC_NAME, ITYPE, FTYPE, VLIM, OFFSET) \
static void \
FUNC_NAME (void *src_dest, int count, int normalize) \
{	FTYPE normfact = normalize ? (VLIM) : 1.0 ; \
		\
	for (int i = 0 ; i < count ; ++ i) \
		* (((FTYPE *) src_dest) + i) = (((FTYPE) (* (((const ITYPE *) src_dest) + i))) - (OFFSET)) / normfact ; \
} /* FUNC_NAME */

#define WAVPACK_GENERATE_SHIFT_FUNC(FUNC_NAME, ITYPE, OTYPE) \
static void \
FUNC_NAME (const ITYPE *src, OTYPE *dest, int count, int shl, int offset) \
{	if (shl == 0 && offset == 0) \
	{	for (int i = 0 ; i < count ; ++ i) \
			dest [i] = (OTYPE) src [i] ; \
		} \
	else if (shl >= 0) \
	{	for (int i = 0 ; i < count ; ++ i) \
			dest [i] = (((OTYPE) src [i]) * (((OTYPE) 1) << shl)) + ((OTYPE) offset) ; \
		} \
	else \
	{	for (int i = 0 ; i < count ; ++ i) \
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
	int is_seek_end ;
} WAVPACK_PRIVATE ;

/*------------------------------------------------------------------------------
** Private static functions and variables.
*/

#define WAVPACK_BUFFER_LENGTH 131072

WAVPACK_GENERATE_FLOAT_TO_INT_FUNC (wavpack_cvtf32i32, int32_t, float, lrintf, fmaxf, fminf)
WAVPACK_GENERATE_FLOAT_TO_INT_FUNC (wavpack_cvtf64i32, int32_t, double, lrint, fmax, fmin)
WAVPACK_GENERATE_FLOAT_TO_INT_FUNC (wavpack_cvtf32i16, int16_t, float, lrintf, fmaxf, fminf)

WAVPACK_GENERATE_FLOAT_TO_INT_FUNC_INPLACE (wavpack_cvtf32i32_inplace, int32_t, float, lrintf, fmaxf, fminf)

WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvti16f32, short, float, 32768, 0)

WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvt32i8f32, int32_t, float, 128, 0)
WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvt32i16f32, int32_t, float, 32768, 0)
WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvt32i24f32, int32_t, float, 8388608, 0)
WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvti32f32, int32_t, float, 2147483648, 0)

WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvt32i8f64, int32_t, double, 128, 0)
WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvt32i16f64, int32_t, double, 32768, 0)
WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvt32i24f64, int32_t, double, 8388608, 0)
WAVPACK_GENERATE_INT_TO_FLOAT_FUNC (wavpack_cvti32f64, int32_t, double, 2147483648, 0)

WAVPACK_GENERATE_INT_TO_FLOAT_FUNC_INPLACE (wavpack_cvti32f32_inplace, int32_t, float, 2147483648, 0)

WAVPACK_GENERATE_SHIFT_FUNC (wavpack_shifti32i16, int32_t, int16_t) ;
WAVPACK_GENERATE_SHIFT_FUNC (wavpack_shifti16i32, int16_t, int32_t) ;
WAVPACK_GENERATE_SHIFT_FUNC (wavpack_shifti32i32, int32_t, int32_t) ;


static void		wavpack_cvtf64f32 (const double *src, float *dest, int count) ;
static void		wavpack_cvtf32f64 (const float *src, double *dest, int count) ;

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

static WavpackStreamReader64	wavpack_reader_wrapper =
{	sf_wavpack_read_bytes_callback,
	sf_wavpack_write_bytes_callback,
	sf_wavpack_get_pos_callback,
	sf_wavpack_set_pos_abs_callback,
	sf_wavpack_set_pos_rel_callback,
	sf_wavpack_push_back_byte_callback,
	sf_wavpack_get_length_callback,
	sf_wavpack_can_seek_callback,
	NULL,
	NULL
} ;


/*------------------------------------------------------------------------------
** Public function.
*/

int
wavpack_open	(SF_PRIVATE *psf)
{	// int		subformat ;
	int		error = 0 ;

	WAVPACK_PRIVATE* pwvpk = calloc (1, sizeof (WAVPACK_PRIVATE)) ;
	psf->codec_data = pwvpk ;
	psf->dataoffset = 0 ;

	if (psf->file.mode == SFM_RDWR)
		return SFE_BAD_MODE_RW ;


	if (psf->file.mode == SFM_READ)
	{	if ((error = wavpack_dec_init (psf)))
			return error ;
		} ;

	// subformat = SF_CODEC (psf->sf.format) ;

	if (psf->file.mode == SFM_WRITE)
	{	if ((SF_CONTAINER (psf->sf.format)) != SF_FORMAT_WAVPACK)
		return	SFE_BAD_OPEN_FORMAT ;

		psf->endian = SF_ENDIAN_LITTLE ;
		psf->sf.seekable = 0 ;

		psf->strings.flags = SF_STR_ALLOW_START ;

		if ((error = wavpack_enc_init (psf)))
			return error ;

		/* In an ideal world we would write the header at this point. Unfortunately
		** that would prevent channel mask or string metadata being added so we have to hold off.
		*/
		psf->write_header = wavpack_write_header ;
	} ;

	psf->datalength = psf->filelength ;

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

	return error ;
} /* wavpack_open */

/*------------------------------------------------------------------------------
*/

static int
wavpack_command (SF_PRIVATE *psf, int command, void *UNUSED (data), int UNUSED (datasize))
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	if (pwvpk == NULL)
		return SFE_WAVPACK_DEAD ;

	if (pwvpk->context == NULL)
		return SFE_INTERNAL ;

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
	if (pwvpk == NULL)
		return SFE_WAVPACK_DEAD ;

	// now let's read header
	psf->sf.samplerate = WavpackGetSampleRate (pwvpk->context) ;
	psf->channel_map = wavlike_gen_channel_map (WavpackGetChannelMask (pwvpk->context)) ;
	psf->sf.channels = WavpackGetNumChannels (pwvpk->context) ;
	psf->sf.frames = WavpackGetNumSamples64 (pwvpk->context) ;

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
	{	switch (bit_depth)
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
	}

	psf->dataoffset = psf_ftell (psf) ;

	return SFE_NO_ERROR ;
}

static int
wavpack_dec_init (SF_PRIVATE *psf)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	char error_buf [128] = { 0 } ;
	int err ;

	psf_fseek (psf, 0, SEEK_SET) ;
	if (psf->error != 0)
		return psf->error ;

	if (pwvpk->context)
		WavpackCloseFile (pwvpk->context) ;

	if ((pwvpk->context = WavpackOpenFileInputEx64 (&wavpack_reader_wrapper, psf, NULL, error_buf, pwvpk->config.flags, 0)) == NULL)
	{	psf_log_printf (psf, "Failed to call `WavpackOpenFileInputEx64`: `%s`\n", error_buf) ;
		return SFE_WAVPACK_NEW_DECODER ;
		} ;

	if ((err = wavpack_read_header (psf)) != SFE_NO_ERROR)
	{	psf_log_printf (psf, "Failed to call `wavpack_read_header`, err is %d\n", err) ;
		WavpackCloseFile (pwvpk->context) ;
		return err ;
		} ;

	return SFE_NO_ERROR ;
} /* wavpack_dec_init */

static int
wavpack_enc_init (SF_PRIVATE *psf)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;

	/*
	** Ok some liberty is taken here to use the most commonly used channel masks
	** instead of "no mapping". If you really want to use "no mapping" for 8 channels and less
	** please don't use wavex. (otherwise we'll have to create a new SF_COMMAND)
	*/
	pwvpk->config.num_channels = psf->sf.channels ;
	pwvpk->config.sample_rate = psf->sf.samplerate ;
	if (pwvpk->config.num_channels <= 0 || pwvpk->config.sample_rate <= 0)
		return SFE_INTERNAL ;

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

		default :	/* 0 when in doubt, use direct out, ie NO mapping*/
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
		default : return SFE_UNIMPLEMENTED ;
		} ;

	psf_fseek (psf, 0, SEEK_SET) ;
	if (psf->error != 0)
		return psf->error ;

	if (pwvpk->context)
		WavpackCloseFile (pwvpk->context) ;

	if ((pwvpk->context = WavpackOpenFileOutput (wavpack_reader_wrapper.write_bytes, psf, NULL)) == NULL)
	{	psf_log_printf (psf, "Failed to call `WavpackOpenFileOutput`\n") ;
		return SFE_WAVPACK_NEW_ENCODER ;
		} ;

	return 0 ;
} /* wavpack_enc_init */

static int
wavpack_write_header (SF_PRIVATE *psf, int UNUSED (calc_length))
{	WAVPACK_PRIVATE* pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;

	// wavpack_write_strings (psf, pwvpk) ;

	int64_t total_samples = psf->sf.frames <= 0 ? -1 : psf->sf.frames ;
	if (WavpackSetConfiguration64 (pwvpk->context, &pwvpk->config, total_samples, NULL) == 0)
	{
		psf_log_printf (psf, "Failed to call `WavpackSetConfiguration64`\n") ;
		return SFE_WAVPACK_WRITE_HEADER ;
		} ;
	if (WavpackPackInit (pwvpk->context) == 0)
	{
		psf_log_printf (psf, "Failed to call `WavpackPackInit`\n") ;
		return SFE_WAVPACK_WRITE_HEADER ;
		} ;

	if (psf->error == SFE_NO_ERROR)
		psf->dataoffset = psf_ftell (psf) ;

	/* this function can be called only once */
	psf->write_header = NULL ;

	return psf->error ;
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
			abort () ;
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
		abort () ;

	if (WavpackPackSamples (pwvpk->context, buffer, item_count / psf->sf.channels) == 0)
	{	psf_log_printf (psf, "Failed to pack all samples in `WavpackPackSamples (context, buffer, %ld)`.\n", item_count) ;
		return SFE_INTERNAL ;
		} ;

	*total_packed_count += item_count ;
	return SFE_NO_ERROR ;
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
	{	psf_log_printf (psf, "Failed to unpack samples in `WavpackUnpackSamples (context, buffer, %ld)`.\n", item_count) ;
		return SFE_WAVPACK_UNPACK_SAMPLES ;
	}

	if (otype == SF_FORMAT_DOUBLE)
	{	if (itype == SF_FORMAT_FLOAT)
			wavpack_cvtf32f64 (((const float *) buffer), out_ptr_double, item_count) ;
		else if (itype == SF_FORMAT_PCM_U8 || itype == SF_FORMAT_PCM_S8)
			wavpack_cvt32i8f64 (buffer, out_ptr_double, item_count, 1) ;
		else if (itype == SF_FORMAT_PCM_16)
			wavpack_cvt32i16f64 (buffer, out_ptr_double, item_count, 1) ;
		else if (itype == SF_FORMAT_PCM_24)
			wavpack_cvt32i24f64 (buffer, out_ptr_double, item_count, 1) ;
		else if (itype == SF_FORMAT_PCM_32)
			wavpack_cvti32f64 (buffer, out_ptr_double, item_count, 1) ;
		else
			abort () ;
		}
	else if (otype == SF_FORMAT_FLOAT)
	{	if (itype == SF_FORMAT_FLOAT)
			memcpy (out_ptr_float, buffer, item_count * sizeof (float)) ;
		else if (itype == SF_FORMAT_PCM_U8 || itype == SF_FORMAT_PCM_S8)
			wavpack_cvt32i8f32 (buffer, out_ptr_float, item_count, 1) ;
		else if (itype == SF_FORMAT_PCM_16)
			wavpack_cvt32i16f32 (buffer, out_ptr_float, item_count, 1) ;
		else if (itype == SF_FORMAT_PCM_24)
			wavpack_cvt32i24f32 (buffer, out_ptr_float, item_count, 1) ;
		else if (itype == SF_FORMAT_PCM_32)
			wavpack_cvti32f32 (buffer, out_ptr_float, item_count, 1) ;
		else
			abort () ;
		}
	else if (itype == SF_FORMAT_FLOAT)
	{	if (otype == SF_FORMAT_PCM_32)
			wavpack_cvtf32i32 ((const float *) buffer, out_ptr_int, item_count, 1, vlim, voffset, psf->add_clipping) ;
		else if (otype == SF_FORMAT_PCM_16)
			wavpack_cvtf32i16 ((const float *) buffer, out_ptr_short, item_count, 1, vlim, voffset, psf->add_clipping) ;
		else
			abort () ;
		}
	else if (itype == SF_FORMAT_PCM_16 || itype == SF_FORMAT_PCM_24 || itype == SF_FORMAT_PCM_32 || itype == SF_FORMAT_PCM_U8 || itype == SF_FORMAT_PCM_S8)
	{	if (otype == SF_FORMAT_PCM_32)
			wavpack_shifti32i32 (buffer, out_ptr_int, item_count, shl, voffset) ;
		else if (otype == SF_FORMAT_PCM_16)
			wavpack_shifti32i16 (buffer, out_ptr_short, item_count, shl, voffset) ;
		else
			abort () ;
		}
	else
		abort () ;

	*total_packed_count += item_count ;
	return SFE_NO_ERROR ;
}

static sf_count_t
wavpack_unpack_buffer_any (SF_PRIVATE *psf, void *ptr, sf_count_t sample_count, int otype)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	if (pwvpk == NULL)
	{	psf_log_printf (psf, "wavpack instance is dead") ;
		return 0 ;
	}
	if (pwvpk->is_seek_end)
	{	psf_log_printf (psf, "wavpack seek end") ;
		return 0 ;
	}

	int itype = SF_CODEC (psf->sf.format) ;

	/* fast path */
	if (sample_count <= 2147483647 && (itype == SF_FORMAT_PCM_32 || itype == SF_FORMAT_FLOAT) && (otype == SF_FORMAT_PCM_32 || otype == SF_FORMAT_FLOAT))
	{	if (WavpackUnpackSamples (pwvpk->context, ptr, sample_count / psf->sf.channels) == 0)
			return 0 ;
		if (itype != otype)
		{	if (itype == SF_FORMAT_PCM_32)
				wavpack_cvti32f32_inplace (ptr, sample_count, 1) ;
			else if (itype == SF_FORMAT_FLOAT)
				wavpack_cvtf32i32_inplace (ptr, sample_count, 1, 2147483648.0, 0, psf->add_clipping) ;
			else
				abort () ;
			} ;
		return sample_count ;
		} ;

	/* general path */
	int32_t *buffer = wavpack_ensure_buffer (pwvpk) ;
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
					vlim = 32768.0 ;
					break ;
				default :
					abort () ;
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
					vlim = 2147483648.0 ;
					break ;
				default :
					abort () ;
				} ;
			break ;
		default:
			abort () ;
	} ;


	sf_count_t total_unpacked_count = 0 ;
	/* full chunks */
	for (sf_count_t i_chunk = 0 ; i_chunk < full_chunk_count ; ++ i_chunk)
	{	int err = wavpack_unpack_buffer_single_any
		(	psf, &total_unpacked_count, ptr, i_chunk, chunk_item_count, chunk_item_count, buffer,
			itype, otype, vlim, shl, voffset
			) ;
		if (err != SFE_NO_ERROR)
			return total_unpacked_count ;
		} ;

	/* remain */
	wavpack_unpack_buffer_single_any
	(	psf, &total_unpacked_count, ptr, full_chunk_count, chunk_item_count, remain_item_count, buffer,
		itype, otype, vlim, shl, voffset
		) ;

	return total_unpacked_count ;
}

static sf_count_t
wavpack_pack_buffer_any (SF_PRIVATE *psf, const void *ptr, sf_count_t sample_count, int itype)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	if (pwvpk == NULL)
	{	psf_log_printf (psf, "wavpack instance is dead") ;
		return 0 ;
	}
	if (pwvpk->is_seek_end)
	{	psf_log_printf (psf, "wavpack seek end") ;
		return 0 ;
	}

	int32_t *buffer = wavpack_ensure_buffer (pwvpk) ;
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
					vlim = 128.0 ;
					break ;
				case SF_FORMAT_PCM_16 :
					vlim = 32768.0 ;
					break ;
				case SF_FORMAT_PCM_24 :
					vlim = 8388608.0 ;
					break ;
				case SF_FORMAT_PCM_32 :
					vlim = 2147483648.0 ;
					break ;
				case SF_FORMAT_FLOAT :
					break ;
				default :
					abort () ;
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
					abort () ;
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
					abort () ;
				} ;
			break ;
		default:
			abort () ;
		} ;

	sf_count_t total_packed_count = 0 ;
	/* full chunks */
	for (sf_count_t i_chunk = 0 ; i_chunk < full_chunk_count ; ++ i_chunk)
	{	int err = wavpack_pack_buffer_single_any
		(	psf, &total_packed_count, ptr, i_chunk, chunk_item_count, chunk_item_count, buffer,
			itype, otype, vlim, shl, voffset
			) ;
		if (err != SFE_NO_ERROR)
			return total_packed_count ;
		} ;

	/* remain */
	wavpack_pack_buffer_single_any
	(	psf, &total_packed_count, ptr, full_chunk_count, chunk_item_count, remain_item_count, buffer,
		itype, otype, vlim, shl, voffset
		) ;

	return total_packed_count ;
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
	if (pwvpk == NULL)
		return SFE_WAVPACK_DEAD ;

	int err = SFE_NO_ERROR ;

	if (pwvpk->context)
	{	if (psf->file.mode == SFM_WRITE)
			err = WavpackFlushSamples (pwvpk->context) != 0 ? SFE_NO_ERROR : SFE_WAVPACK_PACK_SAMPLES ;
		pwvpk->context = WavpackCloseFile (pwvpk->context) ;
		} ;

	if (pwvpk->buffer)
	{
		free (pwvpk->buffer) ;
		pwvpk->buffer = NULL ;
		} ;

	free (pwvpk) ;
	psf->codec_data = NULL ;

	return err ;
} /* wavpack_close */

static sf_count_t
wavpack_seek (SF_PRIVATE *psf, int UNUSED (mode), sf_count_t offset)
{	WAVPACK_PRIVATE *pwvpk = (WAVPACK_PRIVATE*) psf->codec_data ;
	if (pwvpk == NULL)
		return SFE_WAVPACK_DEAD ;

	if (pwvpk == NULL)
	{	psf_log_printf (psf, "wavpack instance is dead\n") ;
		return -1 ;
	}

	if (pwvpk->context == NULL)
		abort () ;

	if (psf->dataoffset < 0)
	{	psf->error = SFE_BAD_SEEK ;
		return -1 ;
		} ;

	if (offset == psf->sf.frames)
	{	pwvpk->is_seek_end = 1 ;
		return offset ;
	}

	pwvpk->is_seek_end = 0 ;

	if (psf->file.mode == SFM_READ)
	{	if (WavpackSeekSample64 (pwvpk->context, offset) != 0)
			return offset ;
		} ;

	psf->error = SFE_BAD_SEEK ;
	wavpack_close (psf) ;
	return -1 ;
} /* wavpack_seek */

static int
wavpack_byterate (SF_PRIVATE *psf)
{	if (psf->file.mode == SFM_READ)
		return (psf->datalength * psf->sf.samplerate) / psf->sf.frames ;

	return -1 ;
} /* wavpack_byterate */

static void
wavpack_cvtf64f32 (const double *src, float *dest, int count)
{	for (int i = 0 ; i < count ; ++ i)
		dest [i] = (float) src [i] ;
} /* wavpack_cvtf64f32 */

static void
wavpack_cvtf32f64 (const float *src, double *dest, int count)
{	for (int i = 0 ; i < count ; ++ i)
		dest [i] = (double) src [i] ;
} /* wavpack_cvtf32f64 */

static int32_t
sf_wavpack_read_bytes_callback (void *client_data, void *data, int32_t bcount)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	sf_count_t bytes = psf_fread (data, 1, bcount, psf) ;
	if (psf->error == 0)
		return bytes ;
	else
		return 0 ;
} /* sf_wavpack_read_bytes_callback */

static int32_t
sf_wavpack_write_bytes_callback (void *client_data, void *data, int32_t bcount)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;

	sf_count_t bytes = psf_fwrite (data, 1, bcount, psf) ;
	if (psf->error == 0)
		return bytes ;
	else
		return 0 ;
} /* sf_wavpack_write_bytes_callback */

static int64_t
sf_wavpack_get_pos_callback (void *client_data)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	sf_count_t absolute_byte_offset = psf_ftell (psf) ;
	if (psf->error == 0)
		return absolute_byte_offset ;
	else
		return -1 ;
} /* sf_wavpack_get_pos_callback */

static int
sf_wavpack_set_pos_abs_callback (void *client_data, int64_t pos)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	psf_fseek (psf, pos, SEEK_SET) ;
	if (psf->error == 0)
		return 0 ;
	else
		return -1 ;
} /* sf_wavpack_set_pos_abs_callback */

static int
sf_wavpack_set_pos_rel_callback (void *client_data, int64_t delta, int mode)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	psf_fseek (psf, delta, mode) ;
	if (psf->error == 0)
		return 0 ;
	else
		return -1 ;
} /* sf_wavpack_set_pos_rel_callback */

static int
sf_wavpack_push_back_byte_callback (void *client_data, int c)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;

	// we need not check `c` because wavpack does not modify it
	psf_fseek (psf, -1, SEEK_CUR) ;
	if (psf->error == 0)
		return c ;
	else
		return -1 ;
} /* sf_wavpack_push_back_byte_callback */

static int64_t
sf_wavpack_get_length_callback (void *client_data)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	sf_count_t orig_abs_offset = psf_ftell (psf) ;
	if (psf->error != 0)
		return 0 ;
	sf_count_t file_len = psf_fseek (psf, 0, SEEK_END) ;
	if (psf->error != 0)
		return 0 ;
	psf_fseek (psf, orig_abs_offset, SEEK_SET) ;
	if (psf->error != 0)
		return 0 ;
	return file_len ;
} /* sf_wavpack_get_length_callback */


static int
sf_wavpack_can_seek_callback (void *client_data)
{	SF_PRIVATE *psf = (SF_PRIVATE*) client_data ;
	return psf->sf.seekable ;
} /* sf_wavpack_can_seek_callback */

/*------------------------------------------------------------------------------
*/

#else /* HAVE_WAVPACK */

int
wavpack_open	(SF_PRIVATE *psf)
{	psf_log_printf (psf, "This version of libsndfile was compiled without Wavpack support.\n") ;
	return SFE_UNIMPLEMENTED ;
} /* wavpack_open */

#endif /* HAVE_WAVPACK */
