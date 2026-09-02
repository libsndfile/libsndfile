/*
** Copyright (C) 2007-2014 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (c) 2007 <robs@users.sourceforge.net>
**
** This library is free software; you can redistribute it and/or modify it
** under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2 of the License, or (at
** your option) any later version.
**
** This library is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
** General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this library.  If not, write to the Free Software Foundation,
** Fifth Floor, 51 Franklin Street, Boston, MA 02111-1301, USA.
*/

/* ADPCM: IMA, OKI <==> 16-bit PCM. */

#include "sfconfig.h"

#include <string.h>

/* Set up for libsndfile environment: */
#include "common.h"

#include "ima_oki_adpcm.h"

#define MIN_SAMPLE	-0x8000
#define MAX_SAMPLE	0x7fff

static int const ima_steps [] =	/* ~16-bit precision */
{	7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230,
	253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963,
	1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
	3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442,
	11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
	32767
} ;

static int const oki_steps [] =	/* ~12-bit precision */
{	256, 272, 304, 336, 368, 400, 448, 496, 544, 592, 656, 720, 800, 880, 960,
	1056, 1168, 1280, 1408, 1552, 1712, 1888, 2080, 2288, 2512, 2768, 3040, 3344,
	3680, 4048, 4464, 4912, 5392, 5936, 6528, 7184, 7904, 8704, 9568, 10528,
	11584, 12736, 14016, 15408, 16960, 18656, 20512, 22576, 24832
} ;

static int const step_changes [] = { -1, -1, -1, -1, 2, 4, 6, 8 } ;

void
ima_oki_adpcm_init (IMA_OKI_ADPCM * state, IMA_OKI_ADPCM_TYPE type)
{
	memset (state, 0, sizeof (*state)) ;

	if (type == IMA_OKI_ADPCM_TYPE_IMA)
	{	state->max_step_index = ARRAY_LEN (ima_steps) - 1 ;
		state->steps = ima_steps ;
		state->mask = (~0) ;
		}
	else
	{	state->max_step_index = ARRAY_LEN (oki_steps) - 1 ;
		state->steps = oki_steps ;
		state->mask = arith_shift_left (~0, 4) ;
		} ;

} /* ima_oki_adpcm_init */


int
adpcm_decode (IMA_OKI_ADPCM * state, int code)
{	int s ;

	s = ((code & 7) << 1) | 1 ;
	s = ((state->steps [state->step_index] * s) >> 3) & state->mask ;

	if (code & 8)
		s = -s ;
	s += state->last_output ;

	if (s < MIN_SAMPLE || s > MAX_SAMPLE)
	{	int grace ;

		grace = (state->steps [state->step_index] >> 3) & state->mask ;

		if (s < MIN_SAMPLE - grace || s > MAX_SAMPLE + grace)
			state->errors ++ ;

		s = s < MIN_SAMPLE ? MIN_SAMPLE : MAX_SAMPLE ;
		} ;

	state->step_index += step_changes [code & 7] ;
	state->step_index = SF_MIN (SF_MAX (state->step_index, 0), state->max_step_index) ;
	state->last_output = s ;

	return s ;
} /* adpcm_decode */

int
adpcm_encode (IMA_OKI_ADPCM * state, int sample)
{	int delta, sign = 0, code ;

	delta = sample - state->last_output ;

	if (delta < 0)
	{	sign = 8 ;
		delta = -delta ;
		} ;

	code = 4 * delta / state->steps [state->step_index] ;
	code = sign | SF_MIN (code, 7) ;
	adpcm_decode (state, code) ; /* Update encoder state */

	return code ;
} /* adpcm_encode */


void
ima_oki_adpcm_decode_block	(IMA_OKI_ADPCM * state)
{	unsigned char code ;
	int k ;

	for (k = 0 ; k < state->code_count ; k++)
	{	code = state->codes [k] ;
		state->pcm [2 * k] = adpcm_decode (state, code >> 4) ;
		state->pcm [2 * k + 1] = adpcm_decode (state, code) ;
		} ;

	state->pcm_count = 2 * k ;
} /* ima_oki_adpcm_decode_block */


void
ima_oki_adpcm_encode_block (IMA_OKI_ADPCM * state)
{	unsigned char code ;
	int k ;

	/*
	**	The codec expects an even number of input samples.
	**
	**	Samples should always be passed in even length blocks. If the last block to
	**	be encoded is odd length, extend that block by one zero valued sample.
	*/
	if (state->pcm_count % 2 == 1)
		state->pcm [state->pcm_count ++] = 0 ;

	for (k = 0 ; k < state->pcm_count / 2 ; k++)
	{	code = adpcm_encode (state, state->pcm [2 * k]) << 4 ;
		code |= adpcm_encode (state, state->pcm [2 * k + 1]) ;
		state->codes [k] = code ;
		} ;

	state->code_count = k ;
} /* ima_oki_adpcm_encode_block */
