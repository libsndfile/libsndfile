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

#include <octave/oct.h>

#include "sndfile.h"

#include "format.h"

#define FOUR_GIG 		(0x100000000LL)
#define	BUFFER_FRAMES	8192

DEFUN_DLD (sfwrite, args, nargout , "\
-*- texinfo -*-\n\
@deftypefn {Function File} sfwrite (@var{filename},@var{data},@var{srate},@var{format})\n\
Write a sound file to disk using libsndfile.\n\
\n\
@seealso{wavwrite}\n\
@end deftypefn\n\
")
{	SNDFILE * file ;
	SF_INFO sfinfo ;

    octave_value_list retval ;

    int nargin  = args.length () ;

    /* Bail out if the input parameters are bad. */
    if (nargin != 4 || !args (0).is_string () || !args (1).is_real_matrix ()
			|| !args (2).is_real_scalar () || !args (3).is_string ()
			|| nargout != 0)
	{	print_usage () ;
		return retval ;
    	} ;

    std::string filename = args (0).string_value () ;
    std::string format = args (3).string_value () ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	sfinfo.format = format_of_str (format) ;
	if (sfinfo.format == 0)
	{	error ("Bad format '%s'", format.c_str ()) ;
		return retval ;
		} ;

	sfinfo.samplerate = lrint (args (2).scalar_value ()) ;
	if (sfinfo.samplerate < 1)
	{	error ("Bad sample rate : %d.\n", sfinfo.samplerate) ;
		return retval ;
		} ;

	Matrix data = args (1).matrix_value () ;
	long rows = args (1).rows () ;
	long cols = args (1).columns () ;

	if (cols > rows)
	{	error ("Audio data should have one column per channel, but supplied data "
				"has %ld rows and %ld columns.\n", rows, cols) ;
		return retval ;
		} ;

	sfinfo.channels = cols ;

    if ((file = sf_open (filename.c_str (), SFM_WRITE, &sfinfo)) == NULL)
	{	error ("Couldn't open file %s : %s", filename.c_str (), sf_strerror (NULL)) ;
		return retval ;
    	} ;

	float buffer [BUFFER_FRAMES * sfinfo.channels] ;
	int writecount ;
	long total = 0 ;

	do
	{
		writecount = BUFFER_FRAMES ;

		/* Make sure we don't read more frames than we allocated. */
		if (total + writecount > rows)
			writecount = rows - total ;

		for (unsigned ch = 0 ; ch < sfinfo.channels ; ch++)
		{	for (int k = 0 ; k < writecount ; k++)
				buffer [k * sfinfo.channels + ch] = data (total + k, ch) ;
			} ;

		if (writecount > 0)
			sf_writef_float (file, buffer, writecount) ;

		total += writecount ;
	} while (writecount > 0 && total < rows) ;

    /* Clean up. */
    sf_close (file) ;

    return retval ;
} /* sfwrite */
