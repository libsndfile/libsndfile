/*
** Copyright (C) 2001-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sndfile.h>

#include "utils.h"

#define	BUFFER_LEN		(1<<15)
#define LOG_BUFFER_SIZE	1024


static	void	test_float_peak	(const char *filename, int typemajor) ;

static void		check_logged_peaks (char *buffer) ;

/* Force the start of this buffer to be double aligned. Sparc-solaris will
** choke if its not.
*/
static	double	data [BUFFER_LEN] ;
static	char	log_buffer [LOG_BUFFER_SIZE] ;

int
main (int argc, char *argv [])
{	int		do_all = 0 ;
	int		test_count = 0 ;

	if (argc != 2)
	{	printf ("Usage : %s <test>\n", argv [0]) ;
		printf ("    Where <test> is one of the following:\n") ;
		printf ("           wav  - test WAV file peak chunk\n") ;
		printf ("           aiff - test AIFF file PEAK chunk\n") ;
		printf ("           all  - perform all tests\n") ;
		exit (1) ;
		} ;

	do_all=!strcmp (argv [1], "all") ;

	if (do_all || ! strcmp (argv [1], "wav"))
	{	test_float_peak ("peak_chunk.wav", SF_FORMAT_WAV) ;
		test_count++ ;
		} ;

	if (do_all || ! strcmp (argv [1], "aiff"))
	{	test_float_peak	("peak_chunk.aiff", SF_FORMAT_AIFF) ;
		test_count++ ;
		} ;

	if (test_count == 0)
	{	printf ("Mono : ************************************\n") ;
		printf ("Mono : *  No '%s' test defined.\n", argv [1]) ;
		printf ("Mono : ************************************\n") ;
		return 1 ;
		} ;

	return 0 ;
} /* main */

/*============================================================================================
**	Here are the test functions.
*/

static void
test_float_peak (const char *filename, int typemajor)
{	SNDFILE		*file ;
	SF_INFO		sfinfo ;
	int			k, frames, count ;

	print_test_name ("test_float_peak", filename) ;

	sfinfo.samplerate	= 44100 ;
	sfinfo.format		= (typemajor | SF_FORMAT_FLOAT) ;
	sfinfo.channels		= 4 ;
	sfinfo.frames		= 0 ;

	frames = BUFFER_LEN / sfinfo.channels ;

	/* Create some random data with a peak value of 0.66. */
	for (k = 0 ; k < BUFFER_LEN ; k++)
		data [k] = (rand () % 2000) / 3000.0 ;

	/* Insert some larger peaks a know locations. */
	data [4 * (frames / 8) + 0] = (frames / 8) * 0.01 ;	/* First channel */
	data [4 * (frames / 6) + 1] = (frames / 6) * 0.01 ;	/* Second channel */
	data [4 * (frames / 4) + 2] = (frames / 4) * 0.01 ;	/* Third channel */
	data [4 * (frames / 2) + 3] = (frames / 2) * 0.01 ;	/* Fourth channel */

	if (! (file = sf_open (filename, SFM_WRITE, &sfinfo)))
	{	printf ("Line %d: sf_open_write failed with error : ", __LINE__) ;
		puts (sf_strerror (NULL)) ;
		exit (1) ;
		} ;

	/*	Write the data in four passed. The data is designed so that peaks will
	**	be written in the different calls to sf_write_double ().
	*/
	for (count = 0 ; count < 4 ; count ++)
	{	if ((k = sf_write_double (file, data + count * BUFFER_LEN / 4, BUFFER_LEN / 4)) != BUFFER_LEN / 4)
		{	printf ("Line %d: sf_write_double # %d failed with short write (%d ->%d)\n", __LINE__, count, BUFFER_LEN / 4, k) ;
			exit (1) ;
			} ;
		} ;

	sf_close (file) ;

	if (! (file = sf_open (filename, SFM_READ, &sfinfo)))
	{	printf ("Line %d: sf_open_read failed with error : ", __LINE__) ;
		puts (sf_strerror (NULL)) ;
		exit (1) ;
		} ;

	if (sfinfo.format != (typemajor | SF_FORMAT_FLOAT))
	{	printf ("Line %d: Returned format incorrect (0x%08X => 0x%08X).\n", __LINE__, (typemajor | SF_FORMAT_FLOAT), sfinfo.format) ;
		exit (1) ;
		} ;

	if (sfinfo.frames != frames)
	{	printf ("Line %d: Incorrect number of.frames in file. (%d => %ld)\n", __LINE__, frames, (long) sfinfo.frames) ;
		exit (1) ;
		} ;

	if (sfinfo.channels != 4)
	{	printf ("Line %d: Incorrect number of channels in file.\n", __LINE__) ;
		exit (1) ;
		} ;

	/* Get the log buffer data. */
	log_buffer [0] = 0 ;
	sf_command	(file, SFC_GET_LOG_INFO, log_buffer, LOG_BUFFER_SIZE) ;

	if (strlen (log_buffer) == 0)
	{	printf ("Line %d: Empty log buffer,\n", __LINE__) ;
		exit (1) ;
		} ;

	check_logged_peaks (log_buffer) ;

	sf_close (file) ;

	unlink (filename) ;
	printf ("ok\n") ;
} /* test_float_peak */

static void
check_logged_peaks (char *buffer)
{	char 	*cptr ;
	int		k, chan, channel_count, position ;
	float	value ;

	if (strstr (buffer, "should") || strstr (buffer, "*"))
	{	printf ("Line %d: Something wrong in buffer. Dumping.\n", __LINE__) ;
		puts (buffer) ;
		exit (1) ;
		} ;

	if (! (cptr = strstr (buffer, "Channels")) || sscanf (cptr, "Channels      : %d", &channel_count) != 1)
	{	printf ("Line %d: Couldn't find channel count.\n", __LINE__) ;
		exit (1) ;
		} ;

	if (channel_count != 4)
	{	printf ("Line %d: Wrong channel count (4 ->%d).\n", __LINE__, channel_count) ;
		exit (1) ;
		} ;

	if (! (cptr = strstr (buffer, "Ch   Position       Value")))
	{	printf ("Line %d: Can't find PEAK data.\n", __LINE__) ;
		exit (1) ;
		} ;

	for (k = 0 ; k < channel_count ; k++)
	{	if (! (cptr = strchr (cptr, '\n')))
		{	printf ("Line %d: Got lost.\n", __LINE__) ;
			exit (1) ;
			} ;
		if (sscanf (cptr, "%d %d %f", &chan, &position, &value) != 3)
		{	printf ("Line %d: sscanf failed.\n", __LINE__) ;
			exit (1) ;
			} ;
		if (position == 0)
		{	printf ("Line %d: peak position for channel %d should not be at offset 0.\n", __LINE__, chan) ;
			printf (buffer) ;
			exit (1) ;
			} ;
		if (chan != k || fabs ((position) * 0.01 - value) > 1e-6)
		{	printf ("Line %d: Error : peak value incorrect!\n", __LINE__) ;
			printf (buffer) ;
			printf ("Line %d: %d %f %f\n", __LINE__, chan, position * 0.01, value) ;
			exit (1) ;
			} ;
		cptr ++ ; /* Move past current newline. */
		} ;

} /* check_logged_peaks */
/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: f10ca506-5808-4393-9d58-e3ec201fb7ee
*/
