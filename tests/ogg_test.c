/*
** Copyright (C) 2007 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include "sfconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include	<sndfile.h>

#include	"utils.h"
#include	"generate.h"

#define	SAMPLE_RATE			44100
#define	DATA_LENGTH			(2 * SAMPLE_RATE)


int
main (void)
{	const char * filename = "vorbis.oga" ;

	print_test_name ("ogg test",filename) ;

	puts ("ok") ;

	return 0 ;
} /* main */

