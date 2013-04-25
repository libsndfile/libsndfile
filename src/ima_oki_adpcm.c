/*
** Copyright (C) 2007-2012 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (c) 2007 <robs@users.sourceforge.net>
** Copyright (c) 2013 Chris Rienzo <chris.rienzo@grasshopper.com>
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

#define IMA_MIN_SAMPLE	-0x8000
#define IMA_MAX_SAMPLE	0x7fff
#define OKI_MIN_SAMPLE	-0x800
#define OKI_MAX_SAMPLE	0x7ff

static int const ima_steps [] =	/* ~16-bit precision */
{	7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230,
	253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963,
	1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
	3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442,
	11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
	32767
} ;

static int const oki_steps [] = /* ~12-bit precision */
{	16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60,
	66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209,
	230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658,
	724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552
} ;

static int const step_changes [] = { -1, -1, -1, -1, 2, 4, 6, 8 } ;

void
ima_oki_adpcm_init (IMA_OKI_ADPCM * state, IMA_OKI_ADPCM_TYPE type)
{
	memset (state, 0, sizeof (*state)) ;

	if (type == IMA_OKI_ADPCM_TYPE_IMA)
	{	state->max_step_index = ARRAY_LEN (ima_steps) - 1 ;
		state->steps = ima_steps ;
		state->min_sample = IMA_MIN_SAMPLE ;
		state->max_sample = IMA_MAX_SAMPLE ;
		state->shift = 0 ;
		}
	else
	{	state->max_step_index = ARRAY_LEN (oki_steps) - 1 ;
		state->steps = oki_steps ;
		state->min_sample = OKI_MIN_SAMPLE ;
		state->max_sample = OKI_MAX_SAMPLE ;
		state->shift = 4 ;
		} ;

} /* ima_oki_adpcm_init */


int
adpcm_decode (IMA_OKI_ADPCM * state, int code)
{	int sample ;
	int ss ;
	int d ;

	ss = state->steps [state->step_index] ;
	d = ss >> 3 ;
	if (code & 1)
		d += ss >> 2 ;
	if (code & 2)
		d += ss >> 1 ;
	if (code & 4)
		d += ss ;
	if (code & 8)
 	d = -d ;
	sample = d + state->last_output ;

	if (sample < state->min_sample || sample > state->max_sample)
	{
		if (sample < state->min_sample - d || sample > state->max_sample + d)
			state->errors ++ ;

		sample = sample < state->min_sample ? state->min_sample : state->max_sample ;
		} ;

	state->step_index += step_changes [code & 7] ;
	state->step_index = SF_MIN (SF_MAX (state->step_index, 0), state->max_step_index) ;
	state->last_output = sample ;

	return sample << state->shift ;
} /* adpcm_decode */

int
adpcm_encode (IMA_OKI_ADPCM * state, int sample)
{	int code ;
	int d ;
	int ss ;

	ss = state->steps [state->step_index] ;
	d = (sample >> state->shift) - state->last_output ;
	code = 0 ;
	if (d < 0)
	{	code = 8 ;
		d = -d ;
		} ;
	if (d >= ss)
	{	code |= 4 ;
		d -= ss ;
		} ;
	if (d >= ss >> 1)
	{	code |= 2 ;
		d -= ss >> 1 ;
		} ;
	if (d >= ss >> 2)
		code |= 1 ;

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


#ifdef TEST

static const unsigned char test_codes [] =
{	0x08, 0x08, 0x04, 0x7f, 0x72, 0xf7, 0x9f, 0x7c, 0xd7, 0xbc, 0x7a, 0xa7, 0xb8,
	0x4b, 0x0b, 0x38, 0xf6, 0x9d, 0x7a, 0xd7, 0xbc, 0x7a, 0xd7, 0xa8, 0x6c, 0x81,
	0x98, 0xe4, 0x0e, 0x7a, 0xd7, 0x9e, 0x7b, 0xc7, 0xab, 0x7a, 0x85, 0xc0, 0xb3,
	0x8f, 0x58, 0xd7, 0xad, 0x7a, 0xd7, 0xad, 0x7a, 0x87, 0xd0, 0x2b, 0x0e, 0x48,
	0xd7, 0xad, 0x78, 0xf7, 0xbc, 0x7a, 0xb7, 0xa8, 0x4b, 0x88, 0x18, 0xd5, 0x8d,
	0x6a, 0xa4, 0x98, 0x08, 0x00, 0x80, 0x88,
} ;

static const short test_pcm [] =
{	32, 0, 32, 0, 32, 320, 864, -352, 2288, 4176, -992, 10112, 5344, -16368,
	30192, 2256, -31888, 14672, -7056, -32448, 14112, -1408, -15504, 22944,
	1216, -1600, 21472, -256, 2560, -15376, 928, -1184, -30048, 10304, 992,
	-30032, 16528, 1008, -30016, 16544, -5184, -30576, 15984, 464, -30560,
	16000, 480, -2336, 30992, 3056, -48, 8400, 720, -1600, -29152, -1216, 1888,
	-32768, 13792, -1728, -32752, 13808, 4496, -32176, 14384, -7344, -32736,
	13824, -1696, -21424, 17024, 1504, -1312, 26880, -1056, 2048, -17680, 256,
	-2064, -32768, 1376, -1728, -32752, 13808, -1712, -32736, 13824, -1696,
	-32720, 13840, -1680, -32704, 13856, -1664, -4480, 32752, -1392, 1712,
	15808, -2128, 192, -27360, 576, -2528, -32768, 13792, -1728, -32752, 13808,
	10704, -31600, 14960, -6768, -32160, 14400, -1120, -20848, 17600, 2080,
	-736, 22336, 608, -2208, -4768, 2208, 96, -21072, 9952, 6848, -24176, 16176,
	656, -13440, 9632, 320, -2496, 64, -2256, -144, 1776, 32, 1616, 176, -1136,
} ;


static void
test_oki_adpcm (void)
{
	IMA_OKI_ADPCM adpcm ;
	unsigned char code ;
	int i, j ;

	printf ("    Testing decoder          : ") ;
	fflush (stdout) ;

	ima_oki_adpcm_init (&adpcm, IMA_OKI_ADPCM_TYPE_OKI) ;
	for (i = 0 ; i < ARRAY_LEN (test_codes) ; i++)
		for (j = 0, code = test_codes [i] ; j < 2 ; j++, code <<= 4)
			if (adpcm_decode (&adpcm, code >> 4) != test_pcm [2 * i + j])
			{	printf ("\n\nFail at i = %d, j = %d.\n\n", i, j) ;
				exit (1) ;
				} ;

	puts ("ok") ;

	printf ("    Testing encoder          : ") ;
	fflush (stdout) ;

	ima_oki_adpcm_init (&adpcm, IMA_OKI_ADPCM_TYPE_OKI) ;
	for (i = 0 ; i < ARRAY_LEN (test_pcm) ; i += j)
	{	code = adpcm_encode (&adpcm, test_pcm [i]) ;
		code = (code << 4) | adpcm_encode (&adpcm, test_pcm [i + 1]) ;
		if (code != test_codes [i / 2])
			{	printf ("\n\nFail at i = %d, %d should be %d\n\n", i, code, test_codes [i / 2]) ;
				exit (1) ;
				} ;
		} ;

	puts ("ok") ;
} /* test_oki_adpcm */

static void
test_oki_adpcm_block (void)
{
	IMA_OKI_ADPCM adpcm ;
	int k ;

	if (ARRAY_LEN (adpcm.pcm) < ARRAY_LEN (test_pcm))
	{	printf ("\n\nLine %d : ARRAY_LEN (adpcm->pcm) > ARRAY_LEN (test_pcm) (%d > %d).\n\n", __LINE__, ARRAY_LEN (adpcm.pcm), ARRAY_LEN (test_pcm)) ;
		exit (1) ;
		} ;

	if (ARRAY_LEN (adpcm.codes) < ARRAY_LEN (test_codes))
	{	printf ("\n\nLine %d : ARRAY_LEN (adcodes->codes) > ARRAY_LEN (test_codes).n", __LINE__) ;
		exit (1) ;
		} ;

	printf ("    Testing block encoder    : ") ;
	fflush (stdout) ;

	ima_oki_adpcm_init (&adpcm, IMA_OKI_ADPCM_TYPE_OKI) ;

	memcpy (adpcm.pcm, test_pcm, sizeof (adpcm.pcm [0]) * ARRAY_LEN (test_pcm)) ;
	adpcm.pcm_count = ARRAY_LEN (test_pcm) ;
	adpcm.code_count = 13 ;

	ima_oki_adpcm_encode_block (&adpcm) ;

	if (adpcm.code_count * 2 != ARRAY_LEN (test_pcm))
	{	printf ("\n\nLine %d : %d * 2 != %d\n\n", __LINE__, adpcm.code_count * 2, ARRAY_LEN (test_pcm)) ;
		exit (1) ;
		} ;

	for (k = 0 ; k < ARRAY_LEN (test_codes) ; k++)
		if (adpcm.codes [k] != test_codes [k])
		{	printf ("\n\nLine %d : Fail at k = %d, %d should be %d\n\n", __LINE__, k, adpcm.codes [k], test_codes [k]) ;
			exit (1) ;
			} ;

	puts ("ok") ;

	printf ("    Testing block decoder    : ") ;
	fflush (stdout) ;

	ima_oki_adpcm_init (&adpcm, IMA_OKI_ADPCM_TYPE_OKI) ;

	memcpy (adpcm.codes, test_codes, sizeof (adpcm.codes [0]) * ARRAY_LEN (test_codes)) ;
	adpcm.code_count = ARRAY_LEN (test_codes) ;
	adpcm.pcm_count = 13 ;

	ima_oki_adpcm_decode_block (&adpcm) ;

	if (adpcm.pcm_count != 2 * ARRAY_LEN (test_codes))
	{	printf ("\n\nLine %d : %d * 2 != %d\n\n", __LINE__, adpcm.pcm_count, 2 * ARRAY_LEN (test_codes)) ;
		exit (1) ;
		} ;

	for (k = 0 ; k < ARRAY_LEN (test_pcm) ; k++)
		if (adpcm.pcm [k] != test_pcm [k])
		{	printf ("\n\nLine %d : Fail at i = %d, %d should be %d.\n\n", __LINE__, k, adpcm.pcm [k], test_pcm [k]) ;
			exit (1) ;
			} ;

	puts ("ok") ;
} /* test_oki_adpcm_block */

int
main (void)
{
	test_oki_adpcm () ;
	test_oki_adpcm_block () ;

	return 0 ;
} /* main */

#endif
