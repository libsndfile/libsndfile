/*
** Copyright (C) 1999-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include	"sndfile.h"
#include	"config.h"
#include	"sfendian.h"
#include	"common.h"
#include	"float_cast.h"

#if CPU_IS_LITTLE_ENDIAN
	#define FLOAT32_READ	float32_le_read
	#define FLOAT32_WRITE	float32_le_write
#elif CPU_IS_BIG_ENDIAN
	#define FLOAT32_READ	float32_be_read
	#define FLOAT32_WRITE	float32_be_write
#endif

/*--------------------------------------------------------------------------------------------
**	Processor floating point capabilities. float32_get_capability () returns one of the
**	latter four values.
*/

enum
{	FLOAT_UNKNOWN		= 0x00,
	FLOAT_CAN_RW_LE		= 0x12,
	FLOAT_CAN_RW_BE		= 0x23,
	FLOAT_BROKEN_LE		= 0x34,
	FLOAT_BROKEN_BE		= 0x45
} ;

/*--------------------------------------------------------------------------------------------
**	Prototypes for private functions.
*/

static sf_count_t	host_read_f2s	(SF_PRIVATE *psf, short *ptr, sf_count_t len) ;
static sf_count_t	host_read_f2i	(SF_PRIVATE *psf, int *ptr, sf_count_t len) ;
static sf_count_t	host_read_f	(SF_PRIVATE *psf, float *ptr, sf_count_t len) ;
static sf_count_t	host_read_f2d	(SF_PRIVATE *psf, double *ptr, sf_count_t len) ;

static sf_count_t	host_write_s2f	(SF_PRIVATE *psf, short *ptr, sf_count_t len) ;
static sf_count_t	host_write_i2f	(SF_PRIVATE *psf, int *ptr, sf_count_t len) ;
static sf_count_t	host_write_f	(SF_PRIVATE *psf, float *ptr, sf_count_t len) ;
static sf_count_t	host_write_d2f	(SF_PRIVATE *psf, double *ptr, sf_count_t len) ;

static	void	f2s_array 	(float *src, int count, short *dest) ;
static	void	f2i_array 	(float *src, int count, int *dest) ;
static	void	f2d_array 	(float *src, int count, double *dest) ;

static 	void	s2f_array 	(short *src, float *dest, int count) ;
static 	void	i2f_array 	(int *src, float *dest, int count) ;
static 	void	d2f_array 	(double *src, float *dest, int count) ;

static void		float32_peak_update	(SF_PRIVATE *psf, float *buffer, int count, int indx) ;

static sf_count_t	replace_read_f2s	(SF_PRIVATE *psf, short *ptr, sf_count_t len) ;
static sf_count_t	replace_read_f2i	(SF_PRIVATE *psf, int *ptr, sf_count_t len) ;
static sf_count_t	replace_read_f	(SF_PRIVATE *psf, float *ptr, sf_count_t len) ;
static sf_count_t	replace_read_f2d	(SF_PRIVATE *psf, double *ptr, sf_count_t len) ;

static sf_count_t	replace_write_s2f	(SF_PRIVATE *psf, short *ptr, sf_count_t len) ;
static sf_count_t	replace_write_i2f	(SF_PRIVATE *psf, int *ptr, sf_count_t len) ;
static sf_count_t	replace_write_f	(SF_PRIVATE *psf, float *ptr, sf_count_t len) ;
static sf_count_t	replace_write_d2f	(SF_PRIVATE *psf, double *ptr, sf_count_t len) ;

static	void	bf2f_array (float *buffer, int count) ;
static	void	f2bf_array (float *buffer, int count) ;

static int		float32_get_capability	(SF_PRIVATE *psf) ;

/*--------------------------------------------------------------------------------------------
**	Exported functions.
*/

int
float32_init	(SF_PRIVATE *psf)
{	static int float_caps ;

	float_caps = float32_get_capability (psf) ;

	psf->blockwidth = sizeof (float) * psf->sf.channels ;

	if (psf->mode == SFM_READ || psf->mode == SFM_RDWR)
	{	switch (psf->endian + float_caps)
		{	case (SF_ENDIAN_BIG + FLOAT_CAN_RW_BE) :
					psf->float_endswap = SF_FALSE ;
					psf->read_short		= host_read_f2s ;
					psf->read_int		= host_read_f2i ;
					psf->read_float		= host_read_f ;
					psf->read_double	= host_read_f2d ;
					break ;

			case (SF_ENDIAN_LITTLE + FLOAT_CAN_RW_LE) :
					psf->float_endswap = SF_FALSE ;
					psf->read_short		= host_read_f2s ;
					psf->read_int		= host_read_f2i ;
					psf->read_float		= host_read_f ;
					psf->read_double	= host_read_f2d ;
					break ;

			case (SF_ENDIAN_BIG + FLOAT_CAN_RW_LE) :
					psf->float_endswap = SF_TRUE ;
					psf->read_short		= host_read_f2s ;
					psf->read_int		= host_read_f2i ;
					psf->read_float		= host_read_f ;
					psf->read_double	= host_read_f2d ;
					break ;

			case (SF_ENDIAN_LITTLE + FLOAT_CAN_RW_BE) :
					psf->float_endswap = SF_TRUE ;
					psf->read_short		= host_read_f2s ;
					psf->read_int		= host_read_f2i ;
					psf->read_float		= host_read_f ;
					psf->read_double	= host_read_f2d ;
					break ;

			/* When the CPU is not IEEE compatible. */
			case (SF_ENDIAN_BIG + FLOAT_BROKEN_LE) :
					psf->float_endswap = SF_TRUE ;
					psf->read_short		= replace_read_f2s ;
					psf->read_int		= replace_read_f2i ;
					psf->read_float		= replace_read_f ;
					psf->read_double	= replace_read_f2d ;
					break ;

			case (SF_ENDIAN_LITTLE + FLOAT_BROKEN_LE) :
					psf->float_endswap = SF_FALSE ;
					psf->read_short		= replace_read_f2s ;
					psf->read_int		= replace_read_f2i ;
					psf->read_float		= replace_read_f ;
					psf->read_double	= replace_read_f2d ;
					break ;

			case (SF_ENDIAN_BIG + FLOAT_BROKEN_BE) :
					psf->float_endswap = SF_FALSE ;
					psf->read_short		= replace_read_f2s ;
					psf->read_int		= replace_read_f2i ;
					psf->read_float		= replace_read_f ;
					psf->read_double	= replace_read_f2d ;
					break ;

			case (SF_ENDIAN_LITTLE + FLOAT_BROKEN_BE) :
					psf->float_endswap = SF_TRUE ;
					psf->read_short		= replace_read_f2s ;
					psf->read_int		= replace_read_f2i ;
					psf->read_float		= replace_read_f ;
					psf->read_double	= replace_read_f2d ;
					break ;

			default : break ;
			} ;
		} ;

	if (psf->mode == SFM_WRITE || psf->mode == SFM_RDWR)
	{	switch (psf->endian + float_caps)
		{	case (SF_ENDIAN_LITTLE + FLOAT_CAN_RW_LE) :
					psf->float_endswap = SF_FALSE ;
					psf->write_short	= host_write_s2f ;
					psf->write_int		= host_write_i2f ;
					psf->write_float	= host_write_f ;
					psf->write_double	= host_write_d2f ;
					break ;

			case (SF_ENDIAN_BIG + FLOAT_CAN_RW_BE) :
					psf->float_endswap = SF_FALSE ;
					psf->write_short	= host_write_s2f ;
					psf->write_int		= host_write_i2f ;
					psf->write_float	= host_write_f ;
					psf->write_double	= host_write_d2f ;
					break ;

			case (SF_ENDIAN_BIG + FLOAT_CAN_RW_LE) :
					psf->float_endswap = SF_TRUE ;
					psf->write_short	= host_write_s2f ;
					psf->write_int		= host_write_i2f ;
					psf->write_float	= host_write_f ;
					psf->write_double	= host_write_d2f ;
					break ;

			case (SF_ENDIAN_LITTLE + FLOAT_CAN_RW_BE) :
					psf->float_endswap = SF_TRUE ;
					psf->write_short	= host_write_s2f ;
					psf->write_int		= host_write_i2f ;
					psf->write_float	= host_write_f ;
					psf->write_double	= host_write_d2f ;
					break ;

			/* When the CPU is not IEEE compatible. */
			case (SF_ENDIAN_BIG + FLOAT_BROKEN_LE) :
					psf->float_endswap = SF_TRUE ;
					psf->write_short	= replace_write_s2f ;
					psf->write_int		= replace_write_i2f ;
					psf->write_float	= replace_write_f ;
					psf->write_double	= replace_write_d2f ;
					break ;

			case (SF_ENDIAN_LITTLE + FLOAT_BROKEN_LE) :
					psf->float_endswap = SF_FALSE ;
					psf->write_short	= replace_write_s2f ;
					psf->write_int		= replace_write_i2f ;
					psf->write_float	= replace_write_f ;
					psf->write_double	= replace_write_d2f ;
					break ;

			case (SF_ENDIAN_BIG + FLOAT_BROKEN_BE) :
					psf->float_endswap = SF_FALSE ;
					psf->write_short	= replace_write_s2f ;
					psf->write_int		= replace_write_i2f ;
					psf->write_float	= replace_write_f ;
					psf->write_double	= replace_write_d2f ;
					break ;

			case (SF_ENDIAN_LITTLE + FLOAT_BROKEN_BE) :
					psf->float_endswap = SF_TRUE ;
					psf->write_short	= replace_write_s2f ;
					psf->write_int		= replace_write_i2f ;
					psf->write_float	= replace_write_f ;
					psf->write_double	= replace_write_d2f ;
					break ;

			default : break ;
			} ;
		} ;

	psf->filelength = psf_get_filelen (psf) ;
	psf->datalength = (psf->dataend) ? psf->dataend - psf->dataoffset :
							psf->filelength - psf->dataoffset ;
	psf->sf.frames = psf->datalength / (psf->sf.channels * sizeof (float)) ;

	return 0 ;
} /* float32_init */

float
float32_be_read (unsigned char *cptr)
{	int		exponent, mantissa, negative ;
	float	fvalue ;

	negative = cptr [0] & 0x80 ;
	exponent = ((cptr [0] & 0x7F) << 1) | ((cptr [1] & 0x80) ? 1 : 0) ;
	mantissa = ((cptr [1] & 0x7F) << 16) | (cptr [2] << 8) | (cptr [3]) ;

	if (! (exponent || mantissa))
		return 0.0 ;

	mantissa |= 0x800000 ;
	exponent = exponent ? exponent - 127 : 0 ;

	fvalue = mantissa ? ((float) mantissa) / ((float) 0x800000) : 0.0 ;

	if (negative)
		fvalue *= -1 ;

	if (exponent > 0)
		fvalue *= (1 << exponent) ;
	else if (exponent < 0)
		fvalue /= (1 << abs (exponent)) ;

	return fvalue ;
} /* float32_be_read */

float
float32_le_read (unsigned char *cptr)
{	int		exponent, mantissa, negative ;
	float	fvalue ;

	negative = cptr [3] & 0x80 ;
	exponent = ((cptr [3] & 0x7F) << 1) | ((cptr [2] & 0x80) ? 1 : 0) ;
	mantissa = ((cptr [2] & 0x7F) << 16) | (cptr [1] << 8) | (cptr [0]) ;

	if (! (exponent || mantissa))
		return 0.0 ;

	mantissa |= 0x800000 ;
	exponent = exponent ? exponent - 127 : 0 ;

	fvalue = mantissa ? ((float) mantissa) / ((float) 0x800000) : 0.0 ;

	if (negative)
		fvalue *= -1 ;

	if (exponent > 0)
		fvalue *= (1 << exponent) ;
	else if (exponent < 0)
		fvalue /= (1 << abs (exponent)) ;

	return fvalue ;
} /* float32_le_read */

void
float32_le_write (float in, unsigned char *out)
{	int		exponent, mantissa, negative = 0 ;

	memset (out, 0, sizeof (int)) ;

	if (in == 0.0)
		return ;

	if (in < 0.0)
	{	in *= -1.0 ;
		negative = 1 ;
		} ;

	in = frexp (in, &exponent) ;

	exponent += 126 ;

	in *= (float) 0x1000000 ;
	mantissa = (((int) in) & 0x7FFFFF) ;

	if (negative)
		out [3] |= 0x80 ;

	if (exponent & 0x01)
		out [2] |= 0x80 ;

	out [0] = mantissa & 0xFF ;
	out [1] = (mantissa >> 8) & 0xFF ;
	out [2] |= (mantissa >> 16) & 0x7F ;
	out [3] |= (exponent >> 1) & 0x7F ;

	return ;
} /* float32_le_write */

void
float32_be_write (float in, unsigned char *out)
{	int		exponent, mantissa, negative = 0 ;

	memset (out, 0, sizeof (int)) ;

	if (in == 0.0)
		return ;

	if (in < 0.0)
	{	in *= -1.0 ;
		negative = 1 ;
		} ;

	in = frexp (in, &exponent) ;

	exponent += 126 ;

	in *= (float) 0x1000000 ;
	mantissa = (((int) in) & 0x7FFFFF) ;

	if (negative)
		out [0] |= 0x80 ;

	if (exponent & 0x01)
		out [1] |= 0x80 ;

	out [3] = mantissa & 0xFF ;
	out [2] = (mantissa >> 8) & 0xFF ;
	out [1] |= (mantissa >> 16) & 0x7F ;
	out [0] |= (exponent >> 1) & 0x7F ;

	return ;
} /* float32_be_write */

/*==============================================================================================
**	Private functions.
*/

static void
float32_peak_update	(SF_PRIVATE *psf, float *buffer, int count, int indx)
{	int 	chan ;
	int		k, position ;
	float	fmaxval ;

	for (chan = 0 ; chan < psf->sf.channels ; chan++)
	{	fmaxval = fabs (buffer [chan]) ;
		position = 0 ;
		for (k = chan ; k < count ; k += psf->sf.channels)
			if (fmaxval < fabs (buffer [k]))
			{	fmaxval = fabs (buffer [k]) ;
				position = k ;
				} ;

		if (fmaxval > psf->pchunk->peaks [chan].value)
		{	psf->pchunk->peaks [chan].value = fmaxval ;
			psf->pchunk->peaks [chan].position = psf->write_current + indx + (position / psf->sf.channels) ;
			} ;
		} ;

	return ;
} /* float32_peak_update */

static int
float32_get_capability	(SF_PRIVATE *psf)
{	union
	{	float			f ;
		int				i ;
		unsigned char	c [4] ;
	} data ;

	data.f = (float) 1.23456789 ; /* Some abitrary value. */

	if (! psf->ieee_replace)
	{	/* If this test is true ints and floats are compatible and little endian. */
		if (data.c [0] == 0x52 && data.c [1] == 0x06 && data.c [2] == 0x9e && data.c [3] == 0x3f)
			return FLOAT_CAN_RW_LE ;

		/* If this test is true ints and floats are compatible and big endian. */
		if (data.c [3] == 0x52 && data.c [2] == 0x06 && data.c [1] == 0x9e && data.c [0] == 0x3f)
			return FLOAT_CAN_RW_BE ;
		} ;

	/* Floats are broken. Don't expect reading or writing to be fast. */
	psf_log_printf (psf, "Using IEEE replacement code for float.\n") ;

	return (CPU_IS_LITTLE_ENDIAN) ? FLOAT_BROKEN_LE : FLOAT_BROKEN_BE ;
} /* float32_get_capability */

/*----------------------------------------------------------------------------------------------
*/

static sf_count_t
host_read_f2s	(SF_PRIVATE *psf, short *ptr, sf_count_t len)
{	int			bufferlen, readcount, thisread ;
	sf_count_t	total = 0 ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	readcount = (len >= bufferlen) ? bufferlen : (int) len ;
		thisread = psf_fread (psf->buffer, sizeof (float), readcount, psf) ;

/* Fix me : Need lef2s_array */
		if (psf->float_endswap == SF_TRUE)
			endswap_int_array ((int*) psf->buffer, readcount) ;

		f2s_array ((float*) (psf->buffer), thisread, ptr + total) ;
		total += thisread ;
		if (thisread < readcount)
			break ;
		len -= thisread ;
		} ;

	return total ;
} /* host_read_f2s */

static sf_count_t
host_read_f2i	(SF_PRIVATE *psf, int *ptr, sf_count_t len)
{	int			bufferlen, readcount, thisread ;
	sf_count_t	total = 0 ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	readcount = (len >= bufferlen) ? bufferlen : (int) len ;
		thisread = psf_fread (psf->buffer, sizeof (float), readcount, psf) ;

		if (psf->float_endswap == SF_TRUE)
			endswap_int_array ((int*) psf->buffer, readcount) ;

		f2i_array ((float*) (psf->buffer), thisread, ptr + total) ;
		total += thisread ;
		if (thisread < readcount)
			break ;
		len -= thisread ;
		} ;

	return total ;
} /* host_read_f2i */

static sf_count_t
host_read_f	(SF_PRIVATE *psf, float *ptr, sf_count_t len)
{	int			bufferlen, readcount, thisread ;
	sf_count_t	total = 0 ;

	if (psf->float_endswap != SF_TRUE)
		return psf_fread (ptr, sizeof (float), len, psf) ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	readcount = (len >= bufferlen) ? bufferlen : (int) len ;
		thisread = psf_fread (psf->buffer, sizeof (float), readcount, psf) ;

		endswap_int_copy ((int*) (ptr + total), (int*) psf->buffer, thisread) ;

		total += thisread ;
		if (thisread < readcount)
			break ;
		len -= thisread ;
		} ;

	return total ;
} /* host_read_f */

static sf_count_t
host_read_f2d	(SF_PRIVATE *psf, double *ptr, sf_count_t len)
{	int			bufferlen, readcount, thisread ;
	sf_count_t	total = 0 ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	readcount = (len >= bufferlen) ? bufferlen : (int) len ;
		thisread = psf_fread (psf->buffer, sizeof (float), readcount, psf) ;

		if (psf->float_endswap == SF_TRUE)
			endswap_int_array ((int*) psf->buffer, readcount) ;

/* Fix me : Need lef2d_array */
		f2d_array ((float*) (psf->buffer), thisread, ptr + total) ;
		total += thisread ;
		if (thisread < readcount)
			break ;
		len -= thisread ;
		} ;

	return total ;
} /* host_read_f2d */

static sf_count_t
host_write_s2f	(SF_PRIVATE *psf, short *ptr, sf_count_t len)
{	int			bufferlen, writecount, thiswrite ;
	sf_count_t	total = 0 ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	writecount = (len >= bufferlen) ? bufferlen : (int) len ;
		s2f_array (ptr + total, (float*) (psf->buffer), writecount) ;

		if (psf->has_peak)
			float32_peak_update (psf, (float *) (psf->buffer), writecount, (int) (total / psf->sf.channels)) ;

		if (psf->float_endswap == SF_TRUE)
			endswap_int_array ((int*) psf->buffer, writecount) ;

		thiswrite = psf_fwrite (psf->buffer, sizeof (float), writecount, psf) ;
		total += thiswrite ;
		if (thiswrite < writecount)
			break ;
		len -= thiswrite ;
		} ;

	return total ;
} /* host_write_s2f */

static sf_count_t
host_write_i2f	(SF_PRIVATE *psf, int *ptr, sf_count_t len)
{	int			bufferlen, writecount, thiswrite ;
	sf_count_t	total = 0 ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	writecount = (len >= bufferlen) ? bufferlen : (int) len ;
		i2f_array (ptr + total, (float*) (psf->buffer), writecount) ;

		if (psf->has_peak)
			float32_peak_update (psf, (float *) (psf->buffer), writecount, (int) (total / psf->sf.channels)) ;

		if (psf->float_endswap == SF_TRUE)
			endswap_int_array ((int*) psf->buffer, writecount) ;

		thiswrite = psf_fwrite (psf->buffer, sizeof (float) , writecount, psf) ;
		total += thiswrite ;
		if (thiswrite < writecount)
			break ;
		len -= thiswrite ;
		} ;

	return total ;
} /* host_write_i2f */

static sf_count_t
host_write_f	(SF_PRIVATE *psf, float *ptr, sf_count_t len)
{	int			bufferlen, writecount, thiswrite ;
	sf_count_t	total = 0 ;

	if (psf->has_peak)
		float32_peak_update (psf, ptr, len, 0) ;

	if (psf->float_endswap != SF_TRUE)
		return psf_fwrite (ptr, sizeof (float), len, psf) ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	writecount = (len >= bufferlen) ? bufferlen : (int) len ;

		endswap_int_copy ((int*) psf->buffer, (int*) (ptr + total), writecount) ;

		thiswrite = psf_fwrite (psf->buffer, sizeof (float), writecount, psf) ;
		total += thiswrite ;
		if (thiswrite < writecount)
			break ;
		len -= thiswrite ;
		} ;

	return total ;
} /* host_write_f */

static sf_count_t
host_write_d2f	(SF_PRIVATE *psf, double *ptr, sf_count_t len)
{	int			bufferlen, writecount, thiswrite ;
	sf_count_t	total = 0 ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	writecount = (len >= bufferlen) ? bufferlen : (int) len ;

		d2f_array (ptr + total, (float*) (psf->buffer), writecount) ;

		if (psf->has_peak)
			float32_peak_update (psf, (float *) (psf->buffer), writecount, (int) (total / psf->sf.channels)) ;

		if (psf->float_endswap == SF_TRUE)
			endswap_int_array ((int*) psf->buffer, writecount) ;

		thiswrite = psf_fwrite (psf->buffer, sizeof (float), writecount, psf) ;
		total += thiswrite ;
		if (thiswrite < writecount)
			break ;
		len -= thiswrite ;
		} ;

	return total ;
} /* host_write_d2f */

/*=======================================================================================
*/

static void
f2s_array (float *src, int count, short *dest)
{	while (count)
	{	count -- ;
		dest [count] = lrintf (src [count]) ;
		} ;
} /* f2s_array */

static void
f2i_array (float *src, int count, int *dest)
{	while (count)
	{	count -- ;
		dest [count] = lrintf (src [count]) ;
		} ;
} /* f2i_array */

static void
f2d_array (float *src, int count, double *dest)
{	while (count)
	{	count -- ;
		dest [count] = src [count] ;
		} ;
} /* f2d_array */

static void
s2f_array (short *src, float *dest, int count)
{	while (count)
	{	count -- ;
		dest [count] = src [count] ;
		} ;

} /* s2f_array */

static void
i2f_array (int *src, float *dest, int count)
{	while (count)
	{	count -- ;
		dest [count] = src [count] ;
		} ;
} /* i2f_array */

static void
d2f_array (double *src, float *dest, int count)
{	while (count)
	{	count -- ;
		dest [count] = src [count] ;
		} ;
} /* d2f_array */

/*=======================================================================================
*/

static sf_count_t
replace_read_f2s	(SF_PRIVATE *psf, short *ptr, sf_count_t len)
{	int			bufferlen, readcount, thisread ;
	sf_count_t	total = 0 ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	readcount = (len >= bufferlen) ? bufferlen : (int) len ;
		thisread = psf_fread (psf->buffer, sizeof (float), readcount, psf) ;

		if (psf->float_endswap == SF_TRUE)
			endswap_int_array ((int*) psf->buffer, readcount) ;

		bf2f_array ((float *) (psf->buffer), readcount) ;

		f2s_array ((float*) (psf->buffer), thisread, ptr + total) ;
		total += thisread ;
		if (thisread < readcount)
			break ;
		len -= thisread ;
		} ;

	return total ;
} /* replace_read_f2s */

static sf_count_t
replace_read_f2i	(SF_PRIVATE *psf, int *ptr, sf_count_t len)
{	int			bufferlen, readcount, thisread ;
	sf_count_t	total = 0 ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	readcount = (len >= bufferlen) ? bufferlen : (int) len ;
		thisread = psf_fread (psf->buffer, sizeof (float), readcount, psf) ;

		if (psf->float_endswap == SF_TRUE)
			endswap_int_array ((int*) psf->buffer, readcount) ;

		bf2f_array ((float *) (psf->buffer), readcount) ;

		f2i_array ((float*) (psf->buffer), thisread, ptr + total) ;
		total += thisread ;
		if (thisread < readcount)
			break ;
		len -= thisread ;
		} ;

	return total ;
} /* replace_read_f2i */

static sf_count_t
replace_read_f	(SF_PRIVATE *psf, float *ptr, sf_count_t len)
{	int			bufferlen, readcount, thisread ;
	sf_count_t	total = 0 ;

	/* FIX THIS */

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	readcount = (len >= bufferlen) ? bufferlen : (int) len ;
		thisread = psf_fread (psf->buffer, sizeof (float), readcount, psf) ;

		if (psf->float_endswap == SF_TRUE)
			endswap_int_array ((int*) psf->buffer, readcount) ;

		bf2f_array ((float *) (psf->buffer), readcount) ;

		memcpy (ptr + total, psf->buffer, readcount * sizeof (float)) ;

		total += thisread ;
		if (thisread < readcount)
			break ;
		len -= thisread ;
		} ;

	return total ;
} /* replace_read_f */

static sf_count_t
replace_read_f2d	(SF_PRIVATE *psf, double *ptr, sf_count_t len)
{	int			bufferlen, readcount, thisread ;
	sf_count_t	total = 0 ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	readcount = (len >= bufferlen) ? bufferlen : (int) len ;
		thisread = psf_fread (psf->buffer, sizeof (float), readcount, psf) ;

		if (psf->float_endswap == SF_TRUE)
			endswap_int_array ((int*) psf->buffer, readcount) ;

		bf2f_array ((float *) (psf->buffer), readcount) ;

		f2d_array ((float*) (psf->buffer), thisread, ptr + total) ;
		total += thisread ;
		if (thisread < readcount)
			break ;
		len -= thisread ;
		} ;

	return total ;
} /* replace_read_f2d */

static sf_count_t
replace_write_s2f	(SF_PRIVATE *psf, short *ptr, sf_count_t len)
{	int			writecount, bufferlen, thiswrite ;
	sf_count_t	total = 0 ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	writecount = (len >= bufferlen) ? bufferlen : (int) len ;
		s2f_array (ptr + total, (float*) (psf->buffer), writecount) ;

		if (psf->has_peak)
			float32_peak_update (psf, (float *) (psf->buffer), writecount, (int) (total / psf->sf.channels)) ;

		f2bf_array ((float *) (psf->buffer), writecount) ;

		if (psf->float_endswap == SF_TRUE)
			endswap_int_array ((int*) psf->buffer, writecount) ;

		thiswrite = psf_fwrite (psf->buffer, sizeof (float), writecount, psf) ;
		total += thiswrite ;
		if (thiswrite < writecount)
			break ;
		len -= thiswrite ;
		} ;

	return total ;
} /* replace_write_s2f */

static sf_count_t
replace_write_i2f	(SF_PRIVATE *psf, int *ptr, sf_count_t len)
{	int			writecount, bufferlen, thiswrite ;
	sf_count_t	total = 0 ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	writecount = (len >= bufferlen) ? bufferlen : (int) len ;
		i2f_array (ptr + total, (float*) (psf->buffer), writecount) ;

		if (psf->has_peak)
			float32_peak_update (psf, (float *) (psf->buffer), writecount, (int) (total / psf->sf.channels)) ;

		f2bf_array ((float *) (psf->buffer), writecount) ;

		if (psf->float_endswap == SF_TRUE)
			endswap_int_array ((int*) psf->buffer, writecount) ;

		thiswrite = psf_fwrite (psf->buffer, sizeof (float), writecount, psf) ;
		total += thiswrite ;
		if (thiswrite < writecount)
			break ;
		len -= thiswrite ;
		} ;

	return total ;
} /* replace_write_i2f */

static sf_count_t
replace_write_f	(SF_PRIVATE *psf, float *ptr, sf_count_t len)
{	int			writecount, bufferlen, thiswrite ;
	sf_count_t	total = 0 ;

	/* FIX THIS */
	if (psf->has_peak)
		float32_peak_update (psf, ptr, len, 0) ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	writecount = (len >= bufferlen) ? bufferlen : (int) len ;

		memcpy (psf->buffer, ptr + total, writecount * sizeof (float)) ;

		f2bf_array ((float *) (psf->buffer), writecount) ;

		if (psf->float_endswap == SF_TRUE)
			endswap_int_array ((int*) psf->buffer, writecount) ;

		thiswrite = psf_fwrite (psf->buffer, sizeof (float) , writecount, psf) ;
		total += thiswrite ;
		if (thiswrite < writecount)
			break ;
		len -= thiswrite ;
		} ;

	return total ;
} /* replace_write_f */

static sf_count_t
replace_write_d2f	(SF_PRIVATE *psf, double *ptr, sf_count_t len)
{	int			writecount, bufferlen, thiswrite ;
	sf_count_t	total = 0 ;

	bufferlen = sizeof (psf->buffer) / sizeof (float) ;

	while (len > 0)
	{	writecount = (len >= bufferlen) ? bufferlen : (int) len ;
		d2f_array (ptr + total, (float*) (psf->buffer), writecount) ;

		if (psf->has_peak)
			float32_peak_update (psf, (float *) (psf->buffer), writecount, (int) (total / psf->sf.channels)) ;

		f2bf_array ((float *) (psf->buffer), writecount) ;

		if (psf->float_endswap == SF_TRUE)
			endswap_int_array ((int*) psf->buffer, writecount) ;

		thiswrite = psf_fwrite (psf->buffer, sizeof (float), writecount, psf) ;
		total += thiswrite ;
		if (thiswrite < writecount)
			break ;
		len -= thiswrite ;
		} ;

	return total ;
} /* replace_write_d2f */

/*----------------------------------------------------------------------------------------------
*/

static void
bf2f_array (float *buffer, int count)
{	while (count)
	{	count -- ;
		buffer [count] = FLOAT32_READ ((unsigned char *) (buffer + count)) ;
		} ;
} /* bf2f_array */

static void
f2bf_array (float *buffer, int count)
{	while (count)
	{	count -- ;
		FLOAT32_WRITE (buffer [count], (unsigned char*) (buffer + count)) ;
		} ;
} /* f2bf_array */

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: b6c34917-488c-4145-9648-f4371fc4c889
*/
