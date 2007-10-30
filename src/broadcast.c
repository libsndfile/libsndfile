/*
** Copyright (C) 2006 Paul Davis <paul@linuxaudiosystems.com>
** Copyright (C) 2006, 2007 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include <stdio.h>
#include <string.h>

#include "common.h"

/*
** Allocate and initialize a broadcast info structure.
*/

SF_BROADCAST_INFO*
broadcast_info_alloc (void)
{	SF_BROADCAST_INFO* bext ;

	if ((bext = calloc (1, sizeof (SF_BROADCAST_INFO))) == NULL)
		return NULL ;

	return bext ;
} /* broadcast_info_alloc */

int
broadcast_info_copy (SF_BROADCAST_INFO* dst, SF_BROADCAST_INFO* src)
{	memcpy (dst, src, sizeof (SF_BROADCAST_INFO)) ;

	/* Currently writing this version. */
	dst->version = 1 ;

	return SF_TRUE ;
} /* broadcast_info_copy */

int
broadcast_add_coding_history (SF_BROADCAST_INFO* bext, unsigned int channels, unsigned int samplerate, int format)
{	const char *newline ;
	char chnstr [16], history [2 * sizeof (bext->coding_history)] ;
	int count, width, current_history_len ;

	/*
	**	From : http://www.sr.se/utveckling/tu/bwf/docs/codhist2.htm
	**
	**	Parameter            Variable string <allowed option>                 Unit
	**	==========================================================================================
	**	Coding Algorithm     A=<ANALOGUE, PCM, MPEG1L1, MPEG1L2, MPEG1L3,
	**	                     MPEG2L1, MPEG2L2, MPEG2L3>
	**	Sampling frequency   F=<11000,22050,24000,32000,44100,48000>          [Hz]
	**	Bit-rate             B=<any bit-rate allowed in MPEG 2 (ISO/IEC       [kbit/s per channel]
	**	                     13818-3)>
	**	Word Length          W=<8, 12, 14, 16, 18, 20, 22, 24>                [bits]
	**	Mode                 M=<mono, stereo, dual-mono, joint-stereo>
	**	Text, free string    T=<a free ASCII-text string for in house use.
	**	                     This string should contain no commas (ASCII
	**	                     2Chex). Examples of the contents: ID-No; codec
	**	                     type; A/D type>
	*/

	switch (channels)
	{	case 0 :
			return SF_FALSE ;

		case 1 :
			strncpy (chnstr, "mono", sizeof (chnstr)) ;
			break ;

		case 2 :
			strncpy (chnstr, "stereo", sizeof (chnstr)) ;
			break ;

		default :
			LSF_SNPRINTF (chnstr, sizeof (chnstr), "%uchn", channels) ;
			break ;
		} ;

	switch (format & SF_FORMAT_SUBMASK)
	{	case SF_FORMAT_PCM_U8 :
		case SF_FORMAT_PCM_S8 :
			width = 8 ;
			break ;
		case SF_FORMAT_PCM_16 :
			width = 16 ;
			break ;
		case SF_FORMAT_PCM_24 :
			width = 24 ;
			break ;
		case SF_FORMAT_PCM_32 :
			width = 32 ;
			break ;
		case SF_FORMAT_FLOAT :
			width = 24 ; /* Bits in the mantissa + 1 */
			break ;
		case SF_FORMAT_DOUBLE :
			width = 53 ; /* Bits in the mantissa + 1 */
			break ;
		case SF_FORMAT_ULAW :
		case SF_FORMAT_ALAW :
			width = 12 ;
			break ;
		default :
			width = 42 ;
			break ;
		} ;

	/* Make sure its a terminated C string. */
	bext->coding_history [sizeof (bext->coding_history) - 1] = 0 ;

	current_history_len = strlen (bext->coding_history) ;
	newline = "" ;

	if (current_history_len != 0 && bext->coding_history [current_history_len - 1] != '\n')
		newline = "\n" ;

	memset (history, 0, sizeof (history)) ;
	count = LSF_SNPRINTF (history, sizeof (history), "%s%sF=%u,A=PCM,M=%s,W=%hu,T=%s-%s\n", bext->coding_history, newline, samplerate, chnstr, width, PACKAGE, VERSION) ;

	while (count >= SIGNED_SIZEOF (bext->coding_history))
	{	/* Coding history is too long, delete oldest part. */
		const char *cptr ;

		/* Entries should be delimited by a newline, so find first newline. */
		if ((cptr = strchr (history, '\n')) == NULL)
		{	/* Ooops, fail to safe and leave a message. */
			count = snprintf (history, sizeof (bext->coding_history), "Something went wrong!\n") ;
			break ;
			} ;

		cptr ++ ;
		count -= cptr - history ;
		memmove (history, cptr, count) ;
		} ;

	/* Zero out end of history chunk. */
	memset (history + count, 0, sizeof (history) - count) ;

	/* Now copy from temporary storage to bext->coding_history. */
	memcpy (bext->coding_history, history, sizeof (bext->coding_history)) ;
	count += count & 1 ;
	bext->coding_history_size = count ;

	return SF_TRUE ;
} /* broadcast_add_coding_history */

