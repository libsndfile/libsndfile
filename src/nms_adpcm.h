/*
** Copyright (C) 2019 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (C) 2019 Arthur Taylor <art@ified.ca>
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

#ifndef SNDFILE_NMSADPCM_H
#define SNDFILE_NMSADPCM_H


#define NMS_SAMPLES_PER_BLOCK 160
#define NMS_BLOCK_SHORTS_32 41
#define NMS_BLOCK_SHORTS_24 31
#define NMS_BLOCK_SHORTS_16 21

/* Variable names from ITU G.726 spec */
struct nms_adpcm_state
{	/* Log of the step size multiplier. Operated on by codewords. */
	short yl ;

	/* Quantizer step size multiplier. Generated from yl. */
	short y ;

	/* Coefficients of the pole predictor */
	short a [2] ;

	/* Coefficients of the zero predictor  */
	short b [6] ;

	/* Previous quantized deltas (multiplied by 2^14) */
	short d_q [7] ;

	/* d_q [x] + s_ez [x], used by the pole-predictor for signs only. */
	short p [3] ;

	/* Previous reconstructed signal values. */
	short s_r [2] ;

	/* Zero predictor components of the signal estimate. */
	short s_ez ;

	/* Signal estimate, (including s_ez). */
	short s_e ;

	/* The most recent codeword (enc:generated, dec:inputted) */
	char Ik ;

	char parity ;

	/*
	** Offset into code tables for the bitrate.
	** 2-bit words: +0
	** 3-bit words: +8
	** 4-bit words: +16
	*/
	int t_off ;
} ;

enum nms_enc_type
{	NMS16,
	NMS24,
	NMS32
} ;


void nms_adpcm_codec_init (struct nms_adpcm_state *s, enum nms_enc_type type) ;
uint8_t nms_adpcm_encode_sample (struct nms_adpcm_state *s, int16_t sl) ;
int16_t nms_adpcm_decode_sample (struct nms_adpcm_state *s, uint8_t code) ;

void nms_adpcm_block_unpack_16 (const uint16_t block [], int16_t codewords [], int16_t *rms) ;
void nms_adpcm_block_unpack_24 (const uint16_t block [], int16_t codewords [], int16_t *rms) ;
void nms_adpcm_block_unpack_32 (const uint16_t block [], int16_t codewords [], int16_t *rms) ;

#endif /* SNDFILE_NMSADPCM_H */
