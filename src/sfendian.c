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

#include	"sfendian.h"

/*-----------------------------------------------------------------------------------------------
** Generic functions for performing endian swapping on integer arrays.
*/

void
endswap_short_array (short *ptr, int len)
{

#if 0
	unsigned char *ucptr, temp ;

	ucptr = ((unsigned char *) ptr) + 2 * len ;
	while (len > 0)
	{	len -- ;
		ucptr -= 2 ;
		temp = ucptr [0] ;
		ucptr [0] = ucptr [1] ;
		ucptr [1] = temp ;
		} ;
#else
	short	temp ;
	while (len > 0)
	{	len -- ;
		temp = ptr [len] ;
		ptr [len] = ENDSWAP_SHORT (temp) ;
		}
#endif
} /* endswap_short_array */

void
endswap_int_array (int *ptr, int len)
{
#if 0
	unsigned char *ucptr, temp ;

	ucptr = ((unsigned char *) ptr) + 4 * len ;
	while (len > 0)
	{	len -- ;
		ucptr -= 4 ;

		temp = ucptr [0] ;
		ucptr [0] = ucptr [3] ;
		ucptr [3] = temp ;

		temp = ucptr [1] ;
		ucptr [1] = ucptr [2] ;
		ucptr [2] = temp ;
		} ;
#else
	int temp ;

	while (len > 0)
	{	len -- ;
		temp = ptr [len] ;
		ptr [len] = ENDSWAP_INT (temp) ;
		} ;
#endif
} /* endswap_int_array */

/*	This function assumes that sizeof (long) == 8, but works correctly even
**	is sizeof (long) == 4.
*/
void
endswap_long_array (long *ptr, int len)
{	unsigned char *ucptr, temp ;

	ucptr = (unsigned char *) ptr + 8 * len ;
	while (len > 0)
	{	len -- ;
		ucptr -= 8 ;

		temp = ucptr [0] ;
		ucptr [0] = ucptr [7] ;
		ucptr [7] = temp ;

		temp = ucptr [1] ;
		ucptr [1] = ucptr [6] ;
		ucptr [6] = temp ;

		temp = ucptr [2] ;
		ucptr [2] = ucptr [5] ;
		ucptr [5] = temp ;

		temp = ucptr [3] ;
		ucptr [3] = ucptr [4] ;
		ucptr [4] = temp ;
		} ;
} /* endswap_long_array */

/*========================================================================================
*/

void
endswap_short_copy (short *dest, short *src, int len)
{
#if	0
	char *psrc, *pdest ;

	psrc = ((char *) src) + 2 * len ;
	pdest = ((char *) dest) + 2 * len ;
	while (len > 0)
	{	len -- ;
		psrc -= 2 ;
		pdest -= 2 ;

		pdest [0] = psrc [1] ;
		pdest [1] = psrc [0] ;
		} ;
#else
	while (len > 0)
	{	len -- ;
		dest [len] = ENDSWAP_SHORT (src [len]) ;
		} ;
#endif
} /* endswap_short_copy */

void
endswap_int_copy (int *dest, int *src, int len)
{
#if	0
	char *psrc, *pdest ;

	psrc = ((char *) src) + 4 * len ;
	pdest = ((char *) dest) + 4 * len ;
	while (len > 0)
	{	len -- ;
		psrc -= 4 ;
		pdest -= 4 ;

		pdest [0] = psrc [3] ;
		pdest [1] = psrc [2] ;
		pdest [2] = psrc [1] ;
		pdest [3] = psrc [0] ;
		} ;
#else
	while (len > 0)
	{	len -- ;
		dest [len] = ENDSWAP_INT (src [len]) ;
		} ;
#endif
} /* endswap_int_copy */

/*	This function assumes that sizeof (long) == 8, but works correctly even
**	is sizeof (long) == 4.
*/
void
endswap_long_copy (long *dest, long *src, int len)
{	char *psrc, *pdest ;

	psrc = (char *) src + 8 * len ;
	pdest = (char *) dest + 8 * len ;
	while (len > 0)
	{	len -- ;
		psrc -= 8 ;
		pdest -= 8 ;

		pdest [0] = psrc [7] ;
		pdest [1] = psrc [6] ;
		pdest [2] = psrc [5] ;
		pdest [3] = psrc [4] ;
		pdest [4] = psrc [3] ;
		pdest [5] = psrc [2] ;
		pdest [6] = psrc [1] ;
		pdest [7] = psrc [0] ;
		} ;
} /* endswap_long_copy */

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 4cf6cc94-da4e-4ef5-aa4c-03dc6cd16810
*/
