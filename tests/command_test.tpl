[+ AutoGen5 template c +]
/*
** Copyright (C) 2001-2002 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <math.h>

#include <sndfile.h>

#include "utils.h"

#define	BUFFER_LEN		(1<<10)
#define LOG_BUFFER_SIZE	1024

static	void	float_norm_test		(const char *filename) ;
static	void	double_norm_test	(const char *filename) ;
static	void	format_tests		(void) ;
static	void	calc_peak_test		(int filetype, const char *filename) ;
static	void	truncate_test		(const char *filename, int filetype) ;

/* Force the start of this buffer to be double aligned. Sparc-solaris will
** choke if its not.
*/

static	float	float_data	[BUFFER_LEN] ;
static	double	double_data	[BUFFER_LEN] ;

int
main (int argc, char *argv [])
{	/*-char	*filename ;-*/
	int		do_all = 0 ;
	int		test_count = 0 ;

	if (argc != 2)
	{	printf ("Usage : %s <test>\n", argv [0]) ;
		printf ("    Where <test> is one of the following:\n") ;
		printf ("           ver    - test sf_command (SFC_GETLIB_VERSION)\n") ;
		/*-printf ("           text  - test adding of text strings\n") ;-*/
		printf ("           norm  - test floating point normalisation\n") ;
		printf ("           format - test format string commands\n") ;
		printf ("           peak   - test peak calculation\n") ;
		printf ("           trunc  - test file truncation\n") ;
		printf ("           all   - perform all tests\n") ;
		exit (1) ;
		} ;

	do_all =! strcmp (argv [1], "all") ;

	if (do_all || ! strcmp (argv [1], "ver"))
	{	char buffer [128] ;
		buffer [0] = 0 ;
		sf_command (NULL, SFC_GET_LIB_VERSION, buffer, sizeof (buffer)) ;
		if (strlen (buffer) < 1)
		{	printf ("Line %d: could not retrieve lib version.\n", __LINE__) ;
			exit (1) ;
			} ;
		test_count++ ;
		} ;

	if (do_all || ! strcmp (argv [1], "norm"))
	{	/*	Preliminary float/double normalisation tests. More testing
		**	is done in the program 'floating_point_test'.
		*/
		float_norm_test		("float.wav") ;
		double_norm_test	("double.wav") ;
		test_count++ ;
		} ;

	if (do_all || ! strcmp (argv [1], "peak"))
	{	calc_peak_test (SF_ENDIAN_BIG		| SF_FORMAT_RAW, "be-peak.raw") ;
		calc_peak_test (SF_ENDIAN_LITTLE	| SF_FORMAT_RAW, "le-peak.raw") ;
		test_count++ ;
		} ;

	if (do_all || ! strcmp (argv [1], "format"))
	{	format_tests () ;
		test_count++ ;
		} ;

	if (do_all || ! strcmp (argv [1], "trunc"))
	{	truncate_test ("truncate.raw", SF_FORMAT_RAW | SF_FORMAT_PCM_32) ;
		truncate_test ("truncate.au" , SF_FORMAT_AU | SF_FORMAT_PCM_16) ;
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
float_norm_test (const char *filename)
{	SNDFILE			*file ;
	SF_INFO			sfinfo ;
	unsigned int	k ;

	print_test_name ("float_norm_test", filename) ;

	sfinfo.samplerate	= 44100 ;
	sfinfo.format		= (SF_FORMAT_RAW | SF_FORMAT_PCM_16) ;
	sfinfo.channels		= 1 ;
	sfinfo.frames		= BUFFER_LEN ;

	/* Create float_data with all values being less than 1.0. */
	for (k = 0 ; k < BUFFER_LEN / 2 ; k++)
		float_data [k] = (k + 5) / (2.0 * BUFFER_LEN) ;
	for (k = BUFFER_LEN / 2 ; k < BUFFER_LEN ; k++)
		float_data [k] = (k + 5) ;

	if (! (file = sf_open (filename, SFM_WRITE, &sfinfo)))
	{	printf ("Line %d: sf_open_write failed with error : ", __LINE__) ;
		fflush (stdout) ;
		puts (sf_strerror (NULL)) ;
		exit (1) ;
		} ;

	/* Normalisation is on by default so no need to do anything here. */

	if ((k = sf_write_float (file, float_data, BUFFER_LEN / 2)) != BUFFER_LEN / 2)
	{	printf ("Line %d: sf_write_float failed with short write (%d ->%d)\n", __LINE__, BUFFER_LEN, k) ;
		exit (1) ;
		} ;

	/* Turn normalisation off. */
	sf_command (file, SFC_SET_NORM_FLOAT, NULL, SF_FALSE) ;

	if ((k = sf_write_float (file, float_data + BUFFER_LEN / 2, BUFFER_LEN / 2)) != BUFFER_LEN / 2)
	{	printf ("Line %d: sf_write_float failed with short write (%d ->%d)\n", __LINE__, BUFFER_LEN, k) ;
		exit (1) ;
		} ;

	sf_close (file) ;

	/* sfinfo struct should still contain correct data. */
	if (! (file = sf_open (filename, SFM_READ, &sfinfo)))
	{	printf ("Line %d: sf_open_read failed with error : ", __LINE__) ;
		fflush (stdout) ;
		puts (sf_strerror (NULL)) ;
		exit (1) ;
		} ;

	if (sfinfo.format != (SF_FORMAT_RAW | SF_FORMAT_PCM_16))
	{	printf ("Line %d: Returned format incorrect (0x%08X => 0x%08X).\n", __LINE__, (SF_FORMAT_RAW | SF_FORMAT_PCM_16), sfinfo.format) ;
		exit (1) ;
		} ;

	if (sfinfo.frames != BUFFER_LEN)
	{	printf ("\n\nLine %d: Incorrect number of.frames in file. (%d => %ld)\n", __LINE__, BUFFER_LEN, SF_COUNT_TO_LONG (sfinfo.frames)) ;
		exit (1) ;
		} ;

	if (sfinfo.channels != 1)
	{	printf ("Line %d: Incorrect number of channels in file.\n", __LINE__) ;
		exit (1) ;
		} ;

	/* Read float_data and check that it is normalised (ie default). */
	if ((k = sf_read_float (file, float_data, BUFFER_LEN)) != BUFFER_LEN)
	{	printf ("\n\nLine %d: sf_read_float failed with short read (%d ->%d)\n", __LINE__, BUFFER_LEN, k) ;
		exit (1) ;
		} ;

	for (k = 0 ; k < BUFFER_LEN ; k++)
		if (float_data [k] >= 1.0)
		{	printf ("\n\nLine %d: float_data [%d] == %f which is greater than 1.0\n", __LINE__, k, float_data [k]) ;
			exit (1) ;
			} ;

	/* Seek to start of file, turn normalisation off, read float_data and check again. */
	sf_seek (file, 0, SEEK_SET) ;
	sf_command (file, SFC_SET_NORM_FLOAT, NULL, SF_FALSE) ;

	if ((k = sf_read_float (file, float_data, BUFFER_LEN)) != BUFFER_LEN)
	{	printf ("\n\nLine %d: sf_read_float failed with short read (%d ->%d)\n", __LINE__, BUFFER_LEN, k) ;
		exit (1) ;
		} ;

	for (k = 0 ; k < BUFFER_LEN ; k++)
		if (float_data [k] < 1.0)
		{	printf ("\n\nLine %d: float_data [%d] == %f which is less than 1.0\n", __LINE__, k, float_data [k]) ;
			exit (1) ;
			} ;

	/* Seek to start of file, turn normalisation on, read float_data and do final check. */
	sf_seek (file, 0, SEEK_SET) ;
	sf_command (file, SFC_SET_NORM_FLOAT, NULL, SF_TRUE) ;

	if ((k = sf_read_float (file, float_data, BUFFER_LEN)) != BUFFER_LEN)
	{	printf ("\n\nLine %d: sf_read_float failed with short read (%d ->%d)\n", __LINE__, BUFFER_LEN, k) ;
		exit (1) ;
		} ;

	for (k = 0 ; k < BUFFER_LEN ; k++)
		if (float_data [k] > 1.0)
		{	printf ("\n\nLine %d: float_data [%d] == %f which is greater than 1.0\n", __LINE__, k, float_data [k]) ;
			exit (1) ;
			} ;


	sf_close (file) ;

	unlink (filename) ;

	printf ("ok\n") ;
} /* float_norm_test */

static void
double_norm_test (const char *filename)
{	SNDFILE			*file ;
	SF_INFO			sfinfo ;
	unsigned int	k ;

	print_test_name ("double_norm_test", filename) ;

	sfinfo.samplerate	= 44100 ;
	sfinfo.format		= (SF_FORMAT_RAW | SF_FORMAT_PCM_16) ;
	sfinfo.channels		= 1 ;
	sfinfo.frames		= BUFFER_LEN ;

	/* Create double_data with all values being less than 1.0. */
	for (k = 0 ; k < BUFFER_LEN / 2 ; k++)
		double_data [k] = (k + 5) / (2.0 * BUFFER_LEN) ;
	for (k = BUFFER_LEN / 2 ; k < BUFFER_LEN ; k++)
		double_data [k] = (k + 5) ;

	if (! (file = sf_open (filename, SFM_WRITE, &sfinfo)))
	{	printf ("Line %d: sf_open_write failed with error : ", __LINE__) ;
		fflush (stdout) ;
		puts (sf_strerror (NULL)) ;
		exit (1) ;
		} ;

	/* Normailsation is on by default so no need to do anything here. */
	/*-sf_command (file, "set-norm-double", "true", 0) ;-*/

	if ((k = sf_write_double (file, double_data, BUFFER_LEN / 2)) != BUFFER_LEN / 2)
	{	printf ("Line %d: sf_write_double failed with short write (%d ->%d)\n", __LINE__, BUFFER_LEN, k) ;
		exit (1) ;
		} ;

	/* Turn normalisation off. */
	sf_command (file, SFC_SET_NORM_DOUBLE, NULL, SF_FALSE) ;

	if ((k = sf_write_double (file, double_data + BUFFER_LEN / 2, BUFFER_LEN / 2)) != BUFFER_LEN / 2)
	{	printf ("Line %d: sf_write_double failed with short write (%d ->%d)\n", __LINE__, BUFFER_LEN, k) ;
		exit (1) ;
		} ;

	sf_close (file) ;

	if (! (file = sf_open (filename, SFM_READ, &sfinfo)))
	{	printf ("Line %d: sf_open_read failed with error : ", __LINE__) ;
		fflush (stdout) ;
		puts (sf_strerror (NULL)) ;
		exit (1) ;
		} ;

	if (sfinfo.format != (SF_FORMAT_RAW | SF_FORMAT_PCM_16))
	{	printf ("Line %d: Returned format incorrect (0x%08X => 0x%08X).\n", __LINE__, (SF_FORMAT_RAW | SF_FORMAT_PCM_16), sfinfo.format) ;
		exit (1) ;
		} ;

	if (sfinfo.frames != BUFFER_LEN)
	{	printf ("\n\nLine %d: Incorrect number of.frames in file. (%d => %ld)\n", __LINE__, BUFFER_LEN, SF_COUNT_TO_LONG (sfinfo.frames)) ;
		exit (1) ;
		} ;

	if (sfinfo.channels != 1)
	{	printf ("Line %d: Incorrect number of channels in file.\n", __LINE__) ;
		exit (1) ;
		} ;

	/* Read double_data and check that it is normalised (ie default). */
	if ((k = sf_read_double (file, double_data, BUFFER_LEN)) != BUFFER_LEN)
	{	printf ("\n\nLine %d: sf_read_double failed with short read (%d ->%d)\n", __LINE__, BUFFER_LEN, k) ;
		exit (1) ;
		} ;

	for (k = 0 ; k < BUFFER_LEN ; k++)
		if (double_data [k] >= 1.0)
		{	printf ("\n\nLine %d: double_data [%d] == %f which is greater than 1.0\n", __LINE__, k, double_data [k]) ;
			exit (1) ;
			} ;

	/* Seek to start of file, turn normalisation off, read double_data and check again. */
	sf_seek (file, 0, SEEK_SET) ;
	sf_command (file, SFC_SET_NORM_DOUBLE, NULL, SF_FALSE) ;

	if ((k = sf_read_double (file, double_data, BUFFER_LEN)) != BUFFER_LEN)
	{	printf ("\n\nLine %d: sf_read_double failed with short read (%d ->%d)\n", __LINE__, BUFFER_LEN, k) ;
		exit (1) ;
		} ;

	for (k = 0 ; k < BUFFER_LEN ; k++)
		if (double_data [k] < 1.0)
		{	printf ("\n\nLine %d: double_data [%d] == %f which is less than 1.0\n", __LINE__, k, double_data [k]) ;
			exit (1) ;
			} ;

	/* Seek to start of file, turn normalisation on, read double_data and do final check. */
	sf_seek (file, 0, SEEK_SET) ;
	sf_command (file, SFC_SET_NORM_DOUBLE, NULL, SF_TRUE) ;

	if ((k = sf_read_double (file, double_data, BUFFER_LEN)) != BUFFER_LEN)
	{	printf ("\n\nLine %d: sf_read_double failed with short read (%d ->%d)\n", __LINE__, BUFFER_LEN, k) ;
		exit (1) ;
		} ;

	for (k = 0 ; k < BUFFER_LEN ; k++)
		if (double_data [k] > 1.0)
		{	printf ("\n\nLine %d: double_data [%d] == %f which is greater than 1.0\n", __LINE__, k, double_data [k]) ;
			exit (1) ;
			} ;


	sf_close (file) ;

	unlink (filename) ;

	printf ("ok\n") ;
} /* double_norm_test */

static	void
format_tests	(void)
{	SF_FORMAT_INFO format_info ;
	SF_INFO		sfinfo ;
	const char	*last_name ;
	int 		k, count ;

	print_test_name ("format_tests", "(null)") ;

	/* Clear out SF_INFO struct and set channels > 0. */
	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	sfinfo.channels = 1 ;

	/* First test simple formats. */

	sf_command (NULL, SFC_GET_SIMPLE_FORMAT_COUNT, &count, sizeof (int)) ;

	if (count < 0 || count > 30)
	{	printf ("Line %d: Weird count.\n", __LINE__) ;
		exit (1) ;
		} ;

	format_info.format = 0 ;
	sf_command (NULL, SFC_GET_SIMPLE_FORMAT, &format_info, sizeof (format_info)) ;

	last_name = format_info.name ;
	for (k = 1 ; k < count ; k ++)
	{	format_info.format = k ;
		sf_command (NULL, SFC_GET_SIMPLE_FORMAT, &format_info, sizeof (format_info)) ;
		if (strcmp (last_name, format_info.name) >= 0)
		{	printf ("\n\nLine %d: format names out of sequence `%s' < `%s'.\n", __LINE__, last_name, format_info.name) ;
			exit (1) ;
			} ;
		sfinfo.format = format_info.format ;

		if (! sf_format_check (&sfinfo))
		{	printf ("\n\nLine %d: sf_format_check failed.\n", __LINE__) ;
			printf ("        Name : %s\n", format_info.name) ;
			printf ("        Format      : 0x%X\n", sfinfo.format) ;
			printf ("        Channels    : 0x%X\n", sfinfo.channels) ;
			printf ("        Sample Rate : 0x%X\n", sfinfo.samplerate) ;
			exit (1) ;
			} ;
		last_name = format_info.name ;
		} ;
	format_info.format = 666 ;
	sf_command (NULL, SFC_GET_SIMPLE_FORMAT, &format_info, sizeof (format_info)) ;

	/* Now test major formats. */
	sf_command (NULL, SFC_GET_FORMAT_MAJOR_COUNT, &count, sizeof (int)) ;

	if (count < 0 || count > 30)
	{	printf ("Line %d: Weird count.\n", __LINE__) ;
		exit (1) ;
		} ;

	format_info.format = 0 ;
	sf_command (NULL, SFC_GET_FORMAT_MAJOR, &format_info, sizeof (format_info)) ;

	last_name = format_info.name ;
	for (k = 1 ; k < count ; k ++)
	{	format_info.format = k ;
		sf_command (NULL, SFC_GET_FORMAT_MAJOR, &format_info, sizeof (format_info)) ;
		if (strcmp (last_name, format_info.name) >= 0)
		{	printf ("\n\nLine %d: format names out of sequence (%d) `%s' < `%s'.\n", __LINE__, k, last_name, format_info.name) ;
			exit (1) ;
			} ;

		last_name = format_info.name ;
		} ;
	format_info.format = 666 ;
	sf_command (NULL, SFC_GET_FORMAT_MAJOR, &format_info, sizeof (format_info)) ;

	/* Now test subtype formats. */
	sf_command (NULL, SFC_GET_FORMAT_SUBTYPE_COUNT, &count, sizeof (int)) ;

	if (count < 0 || count > 30)
	{	printf ("Line %d: Weird count.\n", __LINE__) ;
		exit (1) ;
		} ;

	format_info.format = 0 ;
	sf_command (NULL, SFC_GET_FORMAT_SUBTYPE, &format_info, sizeof (format_info)) ;

	last_name = format_info.name ;
	for (k = 1 ; k < count ; k ++)
	{	format_info.format = k ;
		sf_command (NULL, SFC_GET_FORMAT_SUBTYPE, &format_info, sizeof (format_info)) ;
		} ;
	format_info.format = 666 ;
	sf_command (NULL, SFC_GET_FORMAT_SUBTYPE, &format_info, sizeof (format_info)) ;


	printf ("ok\n") ;
} /* format_tests */

static	void
calc_peak_test (int filetype, const char *filename)
{	SNDFILE		*file ;
	SF_INFO		sfinfo ;
	int			k, format ;
	double		peak ;

	print_test_name ("calc_peak_test", filename) ;

	format = (filetype | SF_FORMAT_PCM_16) ;

	sfinfo.samplerate	= 44100 ;
	sfinfo.format		= format ;
	sfinfo.channels		= 1 ;
	sfinfo.frames		= BUFFER_LEN ;

	/* Create double_data with max value of 0.5. */
	for (k = 0 ; k < BUFFER_LEN ; k++)
		double_data [k] = (k + 1) / (2.0 * BUFFER_LEN) ;

	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_TRUE, __LINE__) ;

	test_write_double_or_die (file, 0, double_data, BUFFER_LEN, __LINE__) ;

	sf_close (file) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_TRUE, __LINE__) ;

	if (sfinfo.format != format)
	{	printf ("Line %d: Returned format incorrect (0x%08X => 0x%08X).\n", __LINE__, format, sfinfo.format) ;
		exit (1) ;
		} ;

	if (sfinfo.frames != BUFFER_LEN)
	{	printf ("\n\nLine %d: Incorrect number of.frames in file. (%d => %ld)\n", __LINE__, BUFFER_LEN, SF_COUNT_TO_LONG (sfinfo.frames)) ;
		exit (1) ;
		} ;

	if (sfinfo.channels != 1)
	{	printf ("Line %d: Incorrect number of channels in file.\n", __LINE__) ;
		exit (1) ;
		} ;

	sf_command (file, SFC_CALC_SIGNAL_MAX, &peak, sizeof (peak)) ;
	if (fabs (peak - (1 << 14)) > 1.0)
	{	printf ("Line %d : Peak value should be %d (is %f).\n", __LINE__, (1 << 14), peak) ;
		exit (1) ;
		} ;

	sf_command (file, SFC_CALC_NORM_SIGNAL_MAX, &peak, sizeof (peak)) ;
	if (fabs (peak - 0.5) > 4e-5)
	{	printf ("Line %d : Peak value should be %f (is %f).\n", __LINE__, 0.5, peak) ;
		exit (1) ;
		} ;

	sf_close (file) ;

	format = (filetype | SF_FORMAT_FLOAT) ;
	sfinfo.samplerate	= 44100 ;
	sfinfo.format		= format ;
	sfinfo.channels		= 1 ;
	sfinfo.frames		= BUFFER_LEN ;

	/* Create double_data with max value of 0.5. */
	for (k = 0 ; k < BUFFER_LEN ; k++)
		double_data [k] = (k + 1) / (2.0 * BUFFER_LEN) ;

	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_TRUE, __LINE__) ;

	test_write_double_or_die (file, 0, double_data, BUFFER_LEN, __LINE__) ;

	sf_close (file) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_TRUE, __LINE__) ;

	if (sfinfo.format != format)
	{	printf ("Line %d: Returned format incorrect (0x%08X => 0x%08X).\n", __LINE__, format, sfinfo.format) ;
		exit (1) ;
		} ;

	if (sfinfo.frames != BUFFER_LEN)
	{	printf ("\n\nLine %d: Incorrect number of.frames in file. (%d => %ld)\n", __LINE__, BUFFER_LEN, SF_COUNT_TO_LONG (sfinfo.frames)) ;
		exit (1) ;
		} ;

	if (sfinfo.channels != 1)
	{	printf ("Line %d: Incorrect number of channels in file.\n", __LINE__) ;
		exit (1) ;
		} ;

	sf_command (file, SFC_CALC_SIGNAL_MAX, &peak, sizeof (peak)) ;
	if (fabs (peak - 0.5) > 1e-5)
	{	printf ("Line %d : Peak value should be %f (is %f).\n", __LINE__, 0.5, peak) ;
		exit (1) ;
		} ;

	sf_command (file, SFC_CALC_NORM_SIGNAL_MAX, &peak, sizeof (peak)) ;
	if (fabs (peak - 0.5) > 1e-5)
	{	printf ("Line %d : Peak value should be %f (is %f).\n", __LINE__, 0.5, peak) ;
		exit (1) ;
		} ;

	sf_close (file) ;

	unlink (filename) ;

	printf ("ok\n") ;
} /* calc_peak_test */

static void
truncate_test (const char *filename, int filetype)
{	SNDFILE 	*file ;
	SF_INFO		sfinfo ;
	sf_count_t	len ;
	int			*int_data ;

	print_test_name ("truncate_test", filename) ;

	sfinfo.samplerate	= 11025 ;
	sfinfo.format		= filetype ;
	sfinfo.channels		= 2 ;

	int_data = (int*) double_data ;

	file = test_open_file_or_die (filename, SFM_RDWR, &sfinfo, SF_TRUE, __LINE__) ;

	test_write_int_or_die (file, 0, int_data, BUFFER_LEN, __LINE__) ;

	len = 100 ;
	if (sf_command (file, SFC_FILE_TRUNCATE, &len, sizeof (len)))
	{	printf ("Line %d: sf_command (SFC_FILE_TRUNCATE) returned error.\n", __LINE__) ;
		exit (1) ;
		} ;

	test_seek_or_die (file, 0, SEEK_CUR, len, 2, __LINE__) ;
	test_seek_or_die (file, 0, SEEK_END, len, 2, __LINE__) ;

	sf_close (file) ;

	unlink (filename) ;
	puts ("ok") ;
} /* truncate_test */

[+ COMMENT

 Do not edit or modify anything in this comment block.
 The following line is a file identity tag for the GNU Arch 
 revision control system.

 arch-tag: 59e5d452-8dae-45aa-99aa-b78dc0deba1c

+]
