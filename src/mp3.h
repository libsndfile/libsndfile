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

typedef struct
{	/* Structure for writing via LAME */
	lame_global_flags *gfp ;
	void		*mp3buffer ;
	int		mp3buffer_size ;
	void		*mp3data_l ;
	void		*mp3data_r ;
	FILE		*fout ;
} MP3_PRIVATE ;

#define MP3BUFFER_SIZE	(9000)
#define MP3DATA_SIZE	(1024)

#endif /* SF_SRC_MP3_H */
