/*
** Copyright (C) 2008-2011 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software ; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation ; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY ; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program ; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef SF_SRC_MP3_H

#include "lame.h"
#include "mpg123.h"

typedef struct
{	/* Structure for writing via LAME */
	lame_global_flags *gfp ;
	void		*mp3buffer ;
//	void		*mp3data ;
	int		quality ;
	int		bitrate ;
	int		initialised ;
} MP3_PRIVATE_W ;

typedef struct	/* read controls */
{	hip_t		hgf ;
	short		*left, *right ;
	mp3data_struct	mp3data ;
	int		count ;
	int		start ;
} MP3_PRIVATE_R ;

#define MP3DATA_SIZE	(1024)
#define MP3BUFFER_SIZE	(5 * MP3DATA_SIZE / 4 + 7200)
static int mp3_bitrates [] =
	{ 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0
		} ;

#endif /* SF_SRC_MP3_H */
