[+ AutoGen5 template c +]
/*
** Copyright (C) 2002-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <string.h>
#include <errno.h>

#include "common.h"
#include "sfendian.h"

[+ FOR int_type
+]static void test_endswap_[+ (get "name") +] (void) ;
[+ ENDFOR int_type
+]

int
main (void)
{
[+ FOR int_type
+]	test_endswap_[+ (get "name") +] () ;
[+ ENDFOR int_type
+]
	return 0 ;
} /* main */

/*==============================================================================
** Actual test functions.
*/

[+ FOR int_type +]
static void
test_endswap_[+ (get "name") +] (void)
{	[+ (get "name") +] buffer [4] ;
	int k ;

	printf ("    %-24s : ", "test_endswap_[+ (get "name") +]") ;
	fflush (stdout) ;

	for (k = 0 ; k < ARRAY_LEN (buffer) ; k++)
		buffer [k] = [+ (get "value") +] + k ;

	endswap_[+ (get "name") +]_array (buffer, ARRAY_LEN (buffer)) ;

	for (k = 0 ; k < ARRAY_LEN (buffer) ; k++)
		if (buffer [k] == k + 1 || buffer [k] != [+ (get "swapper") +] ([+ (get "value") +] + k))
		{	printf ("\n\nLine %d : \n\n", __LINE__) ;
			exit (1) ;
			} ;

	endswap_[+ (get "name") +]_array (buffer, ARRAY_LEN (buffer)) ;

	for (k = 0 ; k < ARRAY_LEN (buffer) ; k++)
		if (buffer [k] != [+ (get "value") +] + k)
		{	printf ("\n\nLine %d : \n\n", __LINE__) ;
			exit (1) ;
			} ;

	endswap_[+ (get "name") +]_copy (buffer, buffer, ARRAY_LEN (buffer)) ;

	for (k = 0 ; k < ARRAY_LEN (buffer) ; k++)
		if (buffer [k] == k + 1 || buffer [k] != [+ (get "swapper") +] ([+ (get "value") +] + k))
		{	printf ("\n\nLine %d : \n\n", __LINE__) ;
			exit (1) ;
			} ;

	endswap_[+ (get "name") +]_copy (buffer, buffer, ARRAY_LEN (buffer)) ;

	for (k = 0 ; k < ARRAY_LEN (buffer) ; k++)
		if (buffer [k] != [+ (get "value") +] + k)
		{	printf ("\n\nLine %d : \n\n", __LINE__) ;
			exit (1) ;
			} ;

	puts ("ok") ;
} /* test_endswap_[+ (get "name") +] */
[+ ENDFOR int_type +]


[+ COMMENT
/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: fd8887e8-8202-4f30-a419-0cf01a0e799b
*/
+]
