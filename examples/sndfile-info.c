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

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<math.h>

#include	<sndfile.h>

#define	BUFFER_LEN		(1 << 16)

#if (defined (WIN32) || defined (_WIN32))
#define	snprintf	_snprintf
#endif

static double	data [BUFFER_LEN] ;

static void
print_version (void)
{	char buffer [256] ;

	sf_command (NULL, SFC_GET_LIB_VERSION, buffer, sizeof (buffer)) ;
	printf ("\nVersion : %s\n\n", buffer) ;
} /* print_usage */


static void
print_usage (char *progname)
{	printf ("\nUsage : %s <file> ...\n", progname) ;
	printf ("\nPrints out information about one or more sound files.\n\n") ;
#if (defined (_WIN32) || defined (WIN32))
		printf ("This is a Unix style command line application which\n"
				"should be run in a MSDOS box or Command Shell window.\n\n") ;
		printf ("Sleeping for 5 seconds before exiting.\n\n") ;
		fflush (stdout) ;

		/* This is the officially blessed by microsoft way but I can't get
		** it to link.
		**     Sleep (15) ;
		** Instead, use this:
		*/
		_sleep (5 * 1000) ;
#endif
} /* print_usage */

static double
get_signal_max (SNDFILE *file)
{	double	max, temp ;
	int		readcount, k, save_state ;

	save_state = sf_command (file, SFC_GET_NORM_DOUBLE, NULL, 0) ;
	sf_command (file, SFC_SET_NORM_DOUBLE, NULL, SF_FALSE) ;

	max = 0.0 ;
	while ((readcount = sf_read_double (file, data, BUFFER_LEN)))
	{	for (k = 0 ; k < readcount ; k++)
		{	temp = fabs (data [k]) ;
			if (temp > max)
				max = temp ;
			} ;
		} ;

	sf_command (file, SFC_SET_NORM_DOUBLE, NULL, save_state) ;

	return max ;
} /* get_signal_max */

static double
calc_decibels (SF_INFO * sfinfo, double max)
{	double decibels ;

	switch (sfinfo->format & SF_FORMAT_SUBMASK)
	{	case SF_FORMAT_PCM_U8 :
		case SF_FORMAT_PCM_S8 :
			decibels = max / 0x80 ;
			break ;

		case SF_FORMAT_PCM_16 :
			decibels = max / 0x8000 ;
			break ;

		case SF_FORMAT_PCM_24 :
			decibels = max / 0x800000 ;
			break ;

		case SF_FORMAT_PCM_32 :
			decibels = max / 0x80000000 ;
			break ;

		case SF_FORMAT_FLOAT :
		case SF_FORMAT_DOUBLE :
			decibels = max / 1.0 ;
			break ;

		default :
			decibels = max / 0x8000 ;
			break ;
		} ;

	return 20.0 * log10 (decibels) ;
} /* calc_decibels */

static const char *
generate_duration_str (SF_INFO *sfinfo)
{	static char str [128] ;

	int seconds ;

	memset (str, 0, sizeof (str)) ;

	if (sfinfo->samplerate < 1)
		return NULL ;

	if (sfinfo->frames / sfinfo->samplerate > 0x7FFFFFFF)
		return "unknown" ;

	seconds = sfinfo->frames / sfinfo->samplerate ;

	snprintf (str, sizeof (str) - 1, "%02d:", seconds / 60 / 60) ;

	seconds = seconds % (60 * 60) ;
	snprintf (str + strlen (str), sizeof (str) - strlen (str) - 1, "%02d:", seconds / 60) ;

	seconds = seconds % 60 ;
	snprintf (str + strlen (str), sizeof (str) - strlen (str) - 1, "%02d.", seconds) ;

	seconds = ((1000 * sfinfo->frames) / sfinfo->samplerate) % 1000 ;
	snprintf (str + strlen (str), sizeof (str) - strlen (str) - 1, "%03d", seconds) ;

	return str ;
} /* generate_duration_str */

int
main (int argc, char *argv [])
{	static	char	strbuffer [BUFFER_LEN] ;
	char 		*progname, *infilename ;
	SNDFILE	 	*infile ;
	SF_INFO	 	sfinfo ;
	int			k ;
	double		signal_max, decibels ;

	progname = strrchr (argv [0], '/') ;
	progname = progname ? progname + 1 : argv [0] ;

	print_version () ;

	if (argc < 2)
	{	print_usage (progname) ;
		return 1 ;
		} ;

	for (k = 1 ; k < argc ; k++)
	{	infilename = argv [k] ;

		if (strcmp (infilename, "--help") == 0 || strcmp (infilename, "-h") == 0)
		{	print_usage (progname) ;
			continue ;
			} ;

		sfinfo.format = 0 ;

		infile = sf_open (infilename, SFM_READ, &sfinfo) ;

		printf ("========================================\n") ;
		sf_command (infile, SFC_GET_LOG_INFO, strbuffer, BUFFER_LEN) ;
		puts (strbuffer) ;
		printf ("----------------------------------------\n") ;

		if (! infile)
		{	printf ("Error : Not able to open input file %s.\n", infilename) ;
			fflush (stdout) ;
			memset (data, 0, sizeof (data)) ;
			puts (sf_strerror (NULL)) ;
			}
		else
		{	printf ("Sample Rate : %d\n", sfinfo.samplerate) ;
			if (sfinfo.frames > 0x7FFFFFFF)
				printf ("Frames      : unknown\n") ;
			else
				printf ("Frames      : %ld\n", (long) sfinfo.frames) ;
			printf ("Channels    : %d\n", sfinfo.channels) ;
			printf ("Format      : 0x%08X\n", sfinfo.format) ;
			printf ("Sections    : %d\n", sfinfo.sections) ;
			printf ("Seekable    : %s\n", (sfinfo.seekable ? "TRUE" : "FALSE")) ;
			printf ("Duration    : %s\n", generate_duration_str (&sfinfo)) ;

			/* Do not use sf_signal_max because it doesn work for non-seekable files . */
			signal_max = get_signal_max (infile) ;
			decibels = calc_decibels (&sfinfo, signal_max) ;
			printf ("Signal Max  : %g (%4.2f dB)\n\n", signal_max, decibels) ;
			} ;

		sf_close (infile) ;
		} ;

	return 0 ;
} /* main */

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: f59a05db-a182-41de-aedd-d717ce2bb099
*/
