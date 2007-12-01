/*
** Copyright (c) 2007 robs@users.sourceforge.net
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


typedef struct
{
	/* private: */
	int mask ;
	int last_output ;
	int step_index ;
	int max_step_index ;
	int const * steps ;
	/* public: */
	int errors ;
} adpcm_codec ;

typedef enum
{	ADPCM_IMA,
	ADPCM_OKI
} adpcm_type ;

void adpcm_init		(adpcm_codec * state, adpcm_type type) ;
int	adpcm_decode	(adpcm_codec * state, int /* 0..15 */ code) ;
int	adpcm_encode	(adpcm_codec * state, int /* -32768..32767 */ sample) ;

int	adpcm_decode_block	(adpcm_codec * state, const unsigned char * codes, short * output, int count) ;
int	adpcm_encode_block	(adpcm_codec * state, const short * input, unsigned char * codes, int count) ;
