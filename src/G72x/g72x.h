/*
** Copyright (C) 1999-2001 Erik de Castro Lopo <erikd@mega-nerd.com>
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

/*
** This file is not the same as the original file from Sun Microsystems. Nearly
** all the original definitions and function prototypes that were in the file 
** of this name have been moved to private.h.
*/	

#ifndef G72X_HEADER_FILE
#define	G72X_HEADER_FILE

/* 
** Number of samples per block to process. 
** Must be a common multiple of possible bits per sample : 2, 3, 4, 5 and 8.
*/
#define	G72x_BLOCK_SIZE		(3*5*8)  

/*	
**	Identifiers for the differing kinds of G72x ADPCM codecs.
**	The identifiers also define the number of encoded bits per sample.
*/

enum
{	G723_16_BITS_PER_SAMPLE = 2,
	G723_24_BITS_PER_SAMPLE = 3, 
	G723_40_BITS_PER_SAMPLE = 5, 

	G721_32_BITS_PER_SAMPLE = 4,
	G721_40_BITS_PER_SAMPLE = 5,

	G723_16_SAMPLES_PER_BLOCK = G72x_BLOCK_SIZE,
	G723_24_SAMPLES_PER_BLOCK = G723_24_BITS_PER_SAMPLE * (G72x_BLOCK_SIZE / G723_24_BITS_PER_SAMPLE), 
	G723_40_SAMPLES_PER_BLOCK = G723_40_BITS_PER_SAMPLE * (G72x_BLOCK_SIZE / G723_40_BITS_PER_SAMPLE), 

	G721_32_SAMPLES_PER_BLOCK = G72x_BLOCK_SIZE,
	G721_40_SAMPLES_PER_BLOCK = G721_40_BITS_PER_SAMPLE * (G72x_BLOCK_SIZE / G721_40_BITS_PER_SAMPLE),

	G723_16_BYTES_PER_BLOCK = (G723_16_BITS_PER_SAMPLE * G72x_BLOCK_SIZE) / 8,
	G723_24_BYTES_PER_BLOCK = (G723_24_BITS_PER_SAMPLE * G72x_BLOCK_SIZE) / 8,
	G723_40_BYTES_PER_BLOCK = (G723_40_BITS_PER_SAMPLE * G72x_BLOCK_SIZE) / 8,

	G721_32_BYTES_PER_BLOCK = (G721_32_BITS_PER_SAMPLE * G72x_BLOCK_SIZE) / 8,
	G721_40_BYTES_PER_BLOCK = (G721_40_BITS_PER_SAMPLE * G72x_BLOCK_SIZE) / 8
} ; 

/* 
** This is the public structure for passing data between the caller and
** the G72x encoder and decoder. 
** The private array is used by the encoder and decoder for internal
** state information and should not be changed in any way by the caller.
** When decoding or encoding a stream, the same instance of this struct
** should be used for every call so that the decoder/encoder keeps the
** correct state data between calls.
*/

typedef struct 
{	/* Private data. Don't mess with it. */
	unsigned long	private [256 / sizeof (long)] ;  

	/* Public data. Read only. */
	int				blocksize, max_bytes, samplesperblock, bytesperblock ;

	/* Public data. Read and write. */
	int				blocks, blockcount, samplecount ;
	unsigned char	block	[G72x_BLOCK_SIZE] ;
	short			samples	[G72x_BLOCK_SIZE] ;
} G72x_DATA ;

/* External function definitions. */

int g72x_reader_init (G72x_DATA *data, int codec) ;
int g72x_writer_init (G72x_DATA *data, int codec) ;
/*
**	Initialize the ADPCM state table for the given codec.
**	Return 0 on success, 1 on fail.
*/

int g72x_decode_block (G72x_DATA *data) ;
/*
**	The caller fills data->block with data->bytes bytes before calling the 
**	function. The value data->bytes must be an integer multiple of 
**	data->blocksize and be <= data->max_bytes.
**	When it returns, the caller can read out data->samples samples. 
*/  

int g72x_encode_block (G72x_DATA *data) ;
/*	
**	The caller fills state->samples some integer multiple data->samples_per_block
**	(up to G72x_BLOCK_SIZE) samples before calling the function.
**	When it returns, the caller can read out bytes encoded bytes. 
*/

#endif /* !G72X_HEADER_FILE */
/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 6ca84e5f-f932-4ba1-87ee-37056d921621
*/

