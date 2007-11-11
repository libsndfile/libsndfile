/*
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, write to the Free Software Foundation,
 * Fifth Floor, 51 Franklin Street, Boston, MA 02111-1301, USA.
 */

/* ADPCM: IMA, OKI <==> 16-bit PCM.   (c) 2007 robs@users.sourceforge.net */

#include <string.h>

/* Set up for libsndfile environment: */
#include "common.h"

#include "adpcms.h"

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
adpcm_init (adpcm_codec * state, adpcm_type type)
{
	memset (state, 0, sizeof (*state)) ;

	if (type == ADPCM_IMA)
	{	state->max_step_index = ARRAY_LEN (ima_steps) - 1 ;
		state->steps = ima_steps ;
		state->mask = (~0) ;
		}
	else
	{	state->max_step_index = ARRAY_LEN (oki_steps) - 1 ;
		state->steps = oki_steps ;
		state->mask = (~0) << 4 ;
		} ;

} /* adpcm_init */

#define MIN_SAMPLE	-0x8000
#define MAX_SAMPLE	0x7fff

int
adpcm_decode (adpcm_codec * state, int code)
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
adpcm_encode (adpcm_codec * state, int sample)
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

#ifdef TEST
static void
test_oki_adpcm (void)
{
	static const char codes [] =
	{	8, 8, 4, 127, 114, -9, -97, 124, -41, -68, 122, -89, -72, 75, 11, 56, -10,
		-99, 122, -41, -68, 122, -41, -88, 108, -127, -104, -28, 14, 122, -41, -98,
		123, -57, -85, 122, -123, -64, -77, -113, 88, -41, -83, 122, -41, -83, 122,
		-121, -48, 43, 14, 72, -41, -83, 120, -9, -68, 122, -73, -88, 75, -120, 24,
		-43, -115, 106, -92, -104, 8, 0, -128, -120,
	} ;
	static const short decodes [] =
	{	32, 0, 32, 0, 32, 320, 880, -336, 2304, 4192, -992, 10128, 5360, -16352,
		30208, 2272, -31872, 14688, -7040, -32432, 14128, -1392, -15488, 22960,
		1232, -1584, 21488, -240, 2576, -15360, 960, -1152, -30032, 10320, 1008,
		-30032, 16528, 1008, -30032, 16528, -5200, -30592, 15968, 448, -30592,
		15968, 448, -2368, 30960, 3024, -80, 8384, 704, -1616, -29168, -1232, 1872,
		-32768, 13792, -1728, -32768, 13792, 4480, -32192, 14368, -7360, -32752,
		13808, -1712, -21456, 16992, 1472, -1344, 26848, -1088, 2016, -17728, 208,
		-2112, -32768, 1376, -1728, -32768, 13792, -1728, -32768, 13792, -1728,
		-32768, 13792, -1728, -32768, 13792, -1728, -4544, 32767, -1377, 1727,
		15823, -2113, 207, -27345, 591, -2513, -32768, 13792, -1728, -32768, 13792,
		10688, -31632, 14928, -6800, -32192, 14368, -1152, -20896, 17552, 2032,
		-784, 22288, 560, -2256, -4816, 2176, 64, -21120, 9920, 6816, -24224, 16128,
		608, -13488, 9584, 272, -2544, 16, -2304, -192, 1728, -16, 1568, 128, -1184,
	} ;

	adpcm_codec adpcm ;
	int i, j, code ;

	printf ("    Testing encoder          : ") ;
	fflush (stdout) ;
	adpcm_init (&adpcm, ADPCM_OKI) ;
	for (i = 0 ; i < ARRAY_LEN (codes) ; i++)
		for (j = 0, code = codes [i] ; j < 2 ; j++, code <<= 4)
			if (adpcm_decode (&adpcm, code >> 4) != decodes [2 * i + j])
			{	printf ("\n\nFail at i = %d, j = %d.\n\n", i, j) ;
				exit (1) ;
				} ;
	puts ("ok") ;

	printf ("    Testing decoder          : ") ;
	fflush (stdout) ;
	adpcm_init (&adpcm, ADPCM_OKI) ;
	for (i = 0 ; i < ARRAY_LEN (decodes) ; i += j)
	{	for (j = 0 ; j < 2 ; j++)
			code = (code << 4) | adpcm_encode (&adpcm, decodes [i + j]) ;
		if ((char) code != codes [i / 2])
			{	printf ("\n\nFail at i = %d.\n\n", i) ;
				exit (1) ;
				} ;
		} ;
	puts ("ok") ;

} /* test_oki_adpcm */

int
main (void)
{
	test_oki_adpcm () ;

	return 0 ;
} /* main */

#endif
