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

#ifndef AU_HEADER_FILE
#define	AU_HEADER_FILE


enum
{	AU_H_G721_32	= 200,
	AU_H_G723_24	= 201,
	AU_H_G723_40	= 202
} ;

int	au_g72x_reader_init (SF_PRIVATE *psf, int codec) ;
int	au_g72x_writer_init (SF_PRIVATE *psf, int codec) ;

#endif /* AU_HEADER_FILE */
/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 63affc81-e204-4468-9705-60abe4d10689
*/
