/*
** Copyright (C) 1999-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "config.h"

#include <stdio.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sndfile.h>

#define	BUFFER_SIZE		(1<<15)
#define	SHORT_BUFFER	(256)

int		main (void)
{	const char 	*noerror, *errstr ;
	int			k ;

	noerror = sf_error_number (0) ;

	printf ("Testing to see if all internal error numbers have corresponding error messages :\n") ;

	for (k = 1 ; k < 1000 ; k++)
	{	errstr = sf_error_number (k) ;

		printf ("%6d : %s\n", k, errstr) ;

		if (errstr == noerror)
			break ;
		if (strstr (errstr, "This is a bug in libsndfile."))
			return 1 ;
		} ;

	return 0 ;
} /* main */

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 799eba74-b505-49d9-89a6-22a7f51a31b4
*/
