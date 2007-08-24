/*
** Copyright (C) 2007 Erik de Castro Lopo <erikd@mega-nerd.com>
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

//#include <cstdio>
#include <octave/oct.h>

#include "sndfile.h"

#define FOUR_GIG 	(0x100000000LL)

DEFUN_DLD (sfread, args, nargout , "\
-*- texinfo -*-\n\
@deftypefn {Function File} {@var{I} =} sfread (@var{filename})\n\
Read a sound file from disk using libsndfile.\n\
\n\
@seealso{wavread}\n\
@end deftypefn\n\
") {
    SNDFILE * file ;
	SF_INFO sfinfo ;

    octave_value_list retval ;

    int nargin  = args.length () ;

    /* Bail out if the input parameters are bad. */
    if ((nargin != 1) || !args (0) .is_string () || (nargout < 1))
	{	print_usage () ;
		return retval ;
    	} ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

    std::string filename = args (0) .string_value () ;

    if ((file = sf_open (filename.c_str (), SFM_READ, &sfinfo)) == NULL)
	{	error ("sfread: couldn't open file %s : %s", filename.c_str (), sf_strerror (NULL)) ;
		return retval ;
    	} ;

	if (sfinfo.frames > FOUR_GIG)
		printf ("This is a really huge file (%d Meg).\nYou may run out of memory trying to load it.\n", sfinfo.frames / (1024 * 1024)) ;

    dim_vector dim = dim_vector () ;
    dim.resize (2) ;
    dim (0) = sfinfo.frames ;
    dim (1) = sfinfo.channels ;
    NDArray out = NDArray (dim, 0.0) ;

	float buffer [4096 * sfinfo.channels] ;
	int readcount ;
	sf_count_t total = 0 ;

	do
	{	readcount = sf_read_float (file, buffer, 4096) ;

		/* Make sure we don't read more frames than we allocated. */
		if (total + readcount > sfinfo.frames)
			readcount = sfinfo.frames - total ;

		for (unsigned ch = 0 ; ch < sfinfo.channels ; ch++)
		{	for (int k = 0 ; k < readcount ; k++)
				out (total + k, ch) = buffer [k * sfinfo.channels + ch] ;
			} ;

		total += readcount ;
	} while (readcount > 0 && total < sfinfo.frames) ;

    retval.append (out.squeeze ()) ;

    /* Clean up. */
    sf_close (file) ;

    return retval ;
} /* sfread */
