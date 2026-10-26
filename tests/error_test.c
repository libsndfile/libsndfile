/*
** Copyright (C) 1999-2018 Erik de Castro Lopo <erikd@mega-nerd.com>
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
#include <errno.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#else
#include "sf_unistd.h"
#endif

#include <sndfile.h>

#include "utils.h"

#define	BUFFER_SIZE		(1 << 15)
#define	SHORT_BUFFER	(256)

static void
error_number_test (void)
{	const char 	*noerror, *errstr ;
	int			k ;

	print_test_name ("error_number_test", "") ;

	noerror = sf_error_number (0) ;

	for (k = 1 ; k < 300 ; k++)
	{	errstr = sf_error_number (k) ;

		/* Test for termination condition. */
		if (errstr == noerror)
			break ;

		/* Test for error. */
		if (strstr (errstr, "This is a bug in libsndfile."))
		{	printf ("\n\nError : error number %d : %s\n\n\n", k, errstr) ;
			exit (1) ;
			} ;
		} ;


	puts ("ok") ;
	return ;
} /* error_number_test */

static void
error_value_test (void)
{	static unsigned char aiff_data [0x1b0] =
	{	'F' , 'O' , 'R' , 'M' ,
		0x00, 0x00, 0x01, 0xA8, /* FORM length */

		'A' , 'I' , 'F' , 'C' ,
		} ;

	const char *filename = "error.aiff" ;
	SNDFILE *file ;
	SF_INFO sfinfo ;
	int error_num ;

	print_test_name ("error_value_test", filename) ;

	dump_data_to_file (filename, aiff_data, sizeof (aiff_data)) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	file = sf_open (filename, SFM_READ, &sfinfo) ;
	if (file != NULL)
	{	printf ("\n\nLine %d : Should not have been able to open this file.\n\n", __LINE__) ;
		exit (1) ;
		} ;

	if ((error_num = sf_error (NULL)) <= 1 || error_num > 300)
	{	printf ("\n\nLine %d : Should not have had an error number of %d.\n\n", __LINE__, error_num) ;
		exit (1) ;
		} ;

	remove (filename) ;
	puts ("ok") ;
	return ;
} /* error_value_test */

static void
no_file_test (const char * filename)
{	SNDFILE		*sndfile ;
	SF_INFO		sfinfo ;

	print_test_name (__func__, filename) ;

	unlink (filename) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	sndfile = sf_open (filename, SFM_READ, &sfinfo) ;

	exit_if_true (sndfile != NULL, "\n\nLine %d : should not have received a valid SNDFILE* pointer.\n", __LINE__) ;

	unlink (filename) ;
	puts ("ok") ;
} /* no_file_test */

static void
zero_length_test (const char *filename)
{	SNDFILE		*sndfile ;
	SF_INFO		sfinfo ;
	FILE		*file ;

	print_test_name (__func__, filename) ;

	/* Create a zero length file. */
	file = fopen (filename, "w") ;
	exit_if_true (file == NULL, "\n\nLine %d : fopen ('%s') failed.\n", __LINE__, filename) ;
	fclose (file) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	sndfile = sf_open (filename, SFM_READ, &sfinfo) ;

	exit_if_true (sndfile != NULL, "\n\nLine %d : should not have received a valid SNDFILE* pointer.\n", __LINE__) ;

	exit_if_true (0 && sf_error (NULL) != SF_ERR_UNRECOGNISED_FORMAT,
		"\n\nLine %3d : Error : %s\n       should be : %s\n", __LINE__,
		sf_strerror (NULL), sf_error_number (SF_ERR_UNRECOGNISED_FORMAT)) ;

	unlink (filename) ;
	puts ("ok") ;
} /* zero_length_test */

static void
bad_wav_test (const char * filename)
{	SNDFILE		*sndfile ;
	SF_INFO		sfinfo ;

	FILE		*file ;
	const char	data [] = "RIFF    WAVEfmt            " ;

	print_test_name (__func__, filename) ;

	if ((file = fopen (filename, "w")) == NULL)
	{	printf ("\n\nLine %d : fopen returned NULL.\n", __LINE__) ;
		exit (1) ;
		} ;

	exit_if_true (fwrite (data, sizeof (data), 1, file) != 1, "\n\nLine %d : fwrite failed.\n", __LINE__) ;
	fclose (file) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	sndfile = sf_open (filename, SFM_READ, &sfinfo) ;

	if (sndfile)
	{	printf ("\n\nLine %d : should not have received a valid SNDFILE* pointer.\n", __LINE__) ;
		exit (1) ;
		} ;

	unlink (filename) ;
	puts ("ok") ;
} /* bad_wav_test */

static void
wav_list_recover_test (const char * filename)
{	SNDFILE		*sndfile ;
	SF_INFO		sfinfo ;

	FILE		*file ;
	const char	data [] =
		"RIFF" "J\000\000\000"
		"WAVE" "fmt " "\020\000\000\000" "\001\000\001\000D\254\000\000\210X\001\000\002\000\020\000"
		"LIST" "\014\000\000\000" "test" "\010\000\000\000" "1234" /* test is 4 bytes short inside LIST container */
		"data" "\004\000\000\000" "abcd" ;

	print_test_name (__func__, filename) ;

	if ((file = fopen (filename, "w")) == NULL)
	{	printf ("\n\nLine %d : fopen returned NULL.\n", __LINE__) ;
		exit (1) ;
		} ;

	exit_if_true (fwrite (data, sizeof (data) - 1, 1, file) != 1, "\n\nLine %d : fwrite failed.\n", __LINE__) ;
	fclose (file) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	sndfile = sf_open (filename, SFM_READ, &sfinfo) ;

	if (!sndfile)
	{	printf ("\n\nLine %d : expected recovery from bogus LIST content.\n", __LINE__) ;
		exit (1) ;
		} ;

	if (sfinfo.frames != 2)
	{	printf ("\n\nLine %d : Should have read data chunk with 2 stereo frames, got %ld.\n\n", __LINE__, (long)sfinfo.frames) ;
		exit (1) ;
		} ;

	unlink (filename) ;
	puts ("ok") ;
} /* wav_list_recover_test */

static void
error_close_test (void)
{	static short buffer [SHORT_BUFFER] ;
	const char	*filename = "error_close.wav" ;
	SNDFILE		*sndfile ;
	SF_INFO		sfinfo ;
	FILE		*file ;

	print_test_name (__func__, filename) ;

	/* Open a FILE* from which we will extract a file descriptor. */
	if ((file = fopen (filename, "w")) == NULL)
	{	printf ("\n\nLine %d : fopen returned NULL.\n", __LINE__) ;
		exit (1) ;
		} ;

	/* Set parameters for writing the file. */
	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	sfinfo.channels = 1 ;
	sfinfo.samplerate = 44100 ;
	sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 ;

	sndfile = sf_open_fd (fileno (file), SFM_WRITE, &sfinfo, SF_TRUE) ;
	if (sndfile == NULL)
	{	printf ("\n\nLine %d : sf_open_fd failed : %s\n", __LINE__, sf_strerror (NULL)) ;
		exit (1) ;
		} ;

	test_write_short_or_die (sndfile, 0, buffer, ARRAY_LEN (buffer), __LINE__) ;

	/* Now close the fd associated with file before calling sf_close. */
	fclose (file) ;

	if (sf_close (sndfile) == 0)
	{
		printf ("\n\nLine %d : sf_close should not have returned zero.\n", __LINE__) ;
#if OS_IS_WIN32
		printf ("\nHowever, this is a known bug on windows platform so we'll ignore it.\n\n") ;
#else
		exit (1) ;
#endif
		} ;

	unlink (filename) ;
	puts ("ok") ;
} /* error_close_test */

static void
unrecognised_test (void)
{	const char	*filename = "unrecognised.bin" ;
	SNDFILE		*sndfile ;
	SF_INFO		sfinfo ;
	FILE		*file ;
	int			k ;

	print_test_name (__func__, filename) ;

	file = fopen (filename, "wb") ;
	exit_if_true (file == NULL,
		"\n\nLine %d : fopen ('%s') failed : %s\n", __LINE__, filename, strerror (errno)
		) ;
	fputs ("Unrecognised file", file) ;
	fclose (file) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	sndfile = sf_open (filename, SFM_READ, &sfinfo) ;

	exit_if_true (sndfile != NULL,
		"\n\nLine %d : SNDFILE* pointer (%p) should be NULL.\n", __LINE__, (void *) sndfile
		) ;

	k = sf_error (sndfile) ;
	exit_if_true (k != SF_ERR_UNRECOGNISED_FORMAT,
		"\n\nLine %d : error (%d) should have been SF_ERR_UNRECOGNISED_FORMAT (%d).\n",
		__LINE__, k, SF_ERR_UNRECOGNISED_FORMAT
		) ;

	unlink (filename) ;
	puts ("ok") ;
} /* unrecognised_test */

/*
**	The HTK, SDS and VOC header writers divide by the sample rate, so a file
**	must never reach them with a sample rate of zero - neither from the
**	caller's SF_INFO nor from a header read in SFM_RDWR mode.
*/

static void
zero_samplerate_write_test (void)
{	static const struct
	{	const char	*filename ;
		int			format ;
	} tests [] =
	{	{	"zero_rate.htk",	SF_FORMAT_HTK | SF_FORMAT_PCM_16	},
		{	"zero_rate.sds",	SF_FORMAT_SDS | SF_FORMAT_PCM_16	},
		{	"zero_rate.voc",	SF_FORMAT_VOC | SF_FORMAT_PCM_U8	},
		} ;
	SNDFILE		*sndfile ;
	SF_INFO		sfinfo ;
	unsigned		k ;

	print_test_name (__func__, "") ;

	for (k = 0 ; k < ARRAY_LEN (tests) ; k++)
	{	memset (&sfinfo, 0, sizeof (sfinfo)) ;
		sfinfo.samplerate = 0 ;
		sfinfo.channels = 1 ;
		sfinfo.format = tests [k].format ;

		sndfile = sf_open (tests [k].filename, SFM_WRITE, &sfinfo) ;
		exit_if_true (sndfile != NULL,
			"\n\nLine %d : sf_open ('%s', SFM_WRITE) with samplerate 0 should have failed.\n",
			__LINE__, tests [k].filename
			) ;

		unlink (tests [k].filename) ;
		} ;

	puts ("ok") ;
} /* zero_samplerate_write_test */

static void
bad_header_samplerate_test (void)
{	/* Sample period 20000000, so 10000000 / period truncates to zero. */
	static const unsigned char htk_data [12 + 16] =
	{	0x00, 0x00, 0x00, 0x08,		/* sample count : 2 * 8 + 12 == sizeof (htk_data) */
		0x01, 0x31, 0x2D, 0x00,		/* sample period : 20000000 */
		0x00, 0x02, 0x00, 0x00,		/* marker : waveform, 2 byte samples */
		} ;
	/* An EXTENDED_II block carries the sample rate as it stands. */
	static const unsigned char voc_data [26 + 16 + 100] =
	{	'C', 'r', 'e', 'a', 't', 'i', 'v', 'e', ' ', 'V', 'o', 'i', 'c', 'e',
		' ', 'F', 'i', 'l', 'e', 0x1A,
		0x1A, 0x00,					/* data offset */
		0x14, 0x01,					/* version */
		0x1F, 0x11,					/* checksum */
		0x09,						/* VOC_EXTENDED_II */
		0x70, 0x00, 0x00,			/* block size */
		0x00, 0x00, 0x00, 0x00,		/* sample rate : zero */
		0x08,						/* bit width */
		0x01,						/* channels */
		0x00, 0x00,					/* encoding : 0 => PCM_U8 */
		} ;
	const char	*htk_file = "bad_rate.htk", *voc_file = "bad_rate.voc" ;
	SNDFILE		*sndfile ;
	SF_INFO		sfinfo ;
	int			k ;

	print_test_name (__func__, "") ;

	/* A sample period the reader cannot turn into a usable rate is guessed
	** at, the way a period of zero or less already was.
	*/
	dump_data_to_file (htk_file, htk_data, sizeof (htk_data)) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	sndfile = sf_open (htk_file, SFM_RDWR, &sfinfo) ;
	exit_if_true (sndfile == NULL,
		"\n\nLine %d : sf_open ('%s', SFM_RDWR) failed : %s\n", __LINE__, htk_file, sf_strerror (NULL)
		) ;
	exit_if_true (sfinfo.samplerate < 1,
		"\n\nLine %d : samplerate (%d) should be greater than zero.\n", __LINE__, sfinfo.samplerate
		) ;
	sf_close (sndfile) ;
	unlink (htk_file) ;

	/* A sample rate of zero written out in the header is malformed. */
	dump_data_to_file (voc_file, voc_data, sizeof (voc_data)) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	sndfile = sf_open (voc_file, SFM_RDWR, &sfinfo) ;
	exit_if_true (sndfile != NULL,
		"\n\nLine %d : sf_open ('%s', SFM_RDWR) with a sample rate of zero should have failed.\n",
		__LINE__, voc_file
		) ;

	k = sf_error (NULL) ;
	exit_if_true (k != SF_ERR_MALFORMED_FILE,
		"\n\nLine %d : error (%d) should have been SF_ERR_MALFORMED_FILE (%d).\n",
		__LINE__, k, SF_ERR_MALFORMED_FILE
		) ;

	unlink (voc_file) ;

	puts ("ok") ;
} /* bad_header_samplerate_test */

int
main (void)
{
	error_number_test () ;
	error_value_test () ;

	error_close_test () ;

	no_file_test ("no_file.wav") ;
	zero_length_test ("zero_length.wav") ;
	bad_wav_test ("bad_wav.wav") ;
	wav_list_recover_test ("list_recover.wav") ;

	unrecognised_test () ;

	zero_samplerate_write_test () ;
	bad_header_samplerate_test () ;

	return 0 ;
} /* main */

