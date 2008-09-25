/*
** Copyright (C) 1999-2008 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in
**       the documentation and/or other materials provided with the
**       distribution.
**     * Neither the author nor the names of any contributors may be used
**       to endorse or promote products derived from this software without
**       specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
** TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
** CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
** OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
** WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
** OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
** ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sndfile.h>

#include "common.h"

#define	 BUFFER_LEN	4096

void
sfe_copy_data_fp (SNDFILE *outfile, SNDFILE *infile, int channels)
{	static double	data [BUFFER_LEN], max ;
	int		frames, readcount, k ;

	frames = BUFFER_LEN / channels ;
	readcount = frames ;

	sf_command (infile, SFC_CALC_SIGNAL_MAX, &max, sizeof (max)) ;

	if (max < 1.0)
	{	while (readcount > 0)
		{	readcount = sf_readf_double (infile, data, frames) ;
			sf_writef_double (outfile, data, readcount) ;
			} ;
		}
	else
	{	sf_command (infile, SFC_SET_NORM_DOUBLE, NULL, SF_FALSE) ;

		while (readcount > 0)
		{	readcount = sf_readf_double (infile, data, frames) ;
			for (k = 0 ; k < readcount * channels ; k++)
				data [k] /= max ;
			sf_writef_double (outfile, data, readcount) ;
			} ;
		} ;

	return ;
} /* sfe_copy_data_fp */

void
sfe_copy_data_int (SNDFILE *outfile, SNDFILE *infile, int channels)
{	static int	data [BUFFER_LEN] ;
	int		frames, readcount ;

	frames = BUFFER_LEN / channels ;
	readcount = frames ;

	while (readcount > 0)
	{	readcount = sf_readf_int (infile, data, frames) ;
		sf_writef_int (outfile, data, readcount) ;
		} ;

	return ;
} /* sfe_copy_data_int */

