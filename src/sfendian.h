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

#include	"config.h"

/*
** Many file types (ie WAV, AIFF) use sets of four consecutive bytes as a
** marker indicating different sections of the file.
** The following MAKE_MARKER macro allows th creation of integer constants
** for these markers.
*/

#if (CPU_IS_LITTLE_ENDIAN == 1)
	#define	MAKE_MARKER(a,b,c,d)	((a)|((b)<<8)|((c)<<16)|((d)<<24))
#elif (CPU_IS_BIG_ENDIAN == 1)
	#define	MAKE_MARKER(a,b,c,d)	(((a)<<24)|((b)<<16)|((c)<<8)|(d))
#else
	#error "Target CPU endian-ness unknown. May need to hand edit src/config.h"
#endif

/* wo standard endswap macros. */

#define	ENDSWAP_SHORT(x)	((((x)>>8)&0xFF)+(((x)&0xFF)<<8))
#define	ENDSWAP_INT(x)		((((x)>>24)&0xFF)+(((x)>>8)&0xFF00)+(((x)&0xFF00)<<8)+(((x)&0xFF)<<24))
/*
** Macros to handle reading of data of a specific endian-ness into host endian
** shorts and ints. The single input is an unsigned char* pointer to the start
** of the object. There are two versions of each macro as we need to deal with
** both big and little endian CPUs.
*/

#if (CPU_IS_LITTLE_ENDIAN == 1)
	#define LES2H_SHORT(x)			(x)
	#define LEI2H_INT(x)			(x)

	#define LES2H_SHORT_PTR(x)		((x) [0] + ((x) [1] << 8))
	#define LES2H_INT_PTR(x)		(((x) [0] << 16) + ((x) [1] << 24))

	#define LET2H_SHORT_PTR(x)		((x) [1] + ((x) [2] << 8))
	#define LET2H_INT_PTR(x)		(((x) [0] << 8) + ((x) [1] << 16) + ((x) [2] << 24))

	#define	LEI2H_SHORT_PTR(x)		((x) [2] + ((x) [3] << 8))
	#define	LEI2H_INT_PTR(x)		((x) [0] + ((x) [1] << 8) + ((x) [2] << 16) + ((x) [3] << 24))

	#define BES2H_SHORT(x)			ENDSWAP_SHORT(x)
	#define BEI2H_INT(x)			ENDSWAP_INT(x)

	#define BES2H_SHORT_PTR(x)		(((x) [0] << 8) + (x) [1])
	#define BES2H_INT_PTR(x)		(((x) [0] << 24) + ((x) [1] << 16))

	#define BET2H_SHORT_PTR(x)		(((x) [0] << 8) + (x) [1])
	#define BET2H_INT_PTR(x)		(((x) [0] << 24) + ((x) [1] << 16) + ((x) [2] << 8))

	#define BEI2H_SHORT_PTR(x)		(((x) [0] << 8) + (x) [1])
	#define BEI2H_INT_PTR(x)		(((x) [0] << 24) + ((x) [1] << 16) + ((x) [2] << 8) + (x) [3])

#elif (CPU_IS_BIG_ENDIAN == 1)
	#define LES2H_SHORT(x)			ENDSWAP_SHORT(x)
	#define LEI2H_INT(x)			ENDSWAP_INT(x)

	#define LES2H_SHORT_PTR(x)		((x) [0] + ((x) [1] << 8))
	#define LES2H_INT_PTR(x)		(((x) [0] << 16) + ((x) [1] << 24))

	#define LET2H_SHORT_PTR(x)		((x) [1] + ((x) [2] << 8))
	#define LET2H_INT_PTR(x)		(((x) [0] << 8) + ((x) [1] << 16) + ((x) [2] << 24))

	#define	LEI2H_SHORT_PTR(x)		((x) [2] + ((x) [3] << 8))
	#define	LEI2H_INT_PTR(x)		((x) [0] + ((x) [1] << 8) + ((x) [2] << 16) + ((x) [3] << 24))

	#define BES2H_SHORT(x)			(x)
	#define BEI2H_INT(x)			(x)

	#define BES2H_SHORT_PTR(x)		(((short*) x) [0])
	#define BES2H_INT_PTR(x)		((((short*) x) [0]) << 16)

	#define BET2H_SHORT_PTR(x)		(((x) [0] << 8) + (x) [1])
	#define BET2H_INT_PTR(x)		(((x) [0] << 24) + ((x) [1] << 16) + ((x) [2] << 8))

	#define BEI2H_SHORT_PTR(x)		((((int*) x) [0]) >> 16)
	#define BEI2H_INT_PTR(x)		(((int*) x) [0])

#else
	#error "Target CPU endian-ness unknown. May need to hand edit src/config.h"
#endif

/* Endian swapping routines implemented in sfendian.c. */

void	endswap_short_array	(short *ptr, int len) ;
void	endswap_int_array	(int *ptr, int len) ;

/* Always swaps 8 byte values whether sizeof (long) == 8 or not. */
void	endswap_long_array	(long *ptr, int len) ;

void	endswap_short_copy	(short *dest, short *src, int len) ;
void	endswap_int_copy	(int *dest, int *src, int len) ;

/* Always swaps 8 byte values whether sizeof (long) == 8 or not. */
void	endswap_long_copy	(long *dest, long *src, int len) ;

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: f0c5cd54-42d3-4237-90ec-11fe24995de7
*/
