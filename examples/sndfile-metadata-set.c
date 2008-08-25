/*
** Copyright (C) 2008 George Blood Audio
** Written by Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include <sndfile.h>

#include "copy_data.h"

#define	BUFFER_LEN		(1 << 16)


typedef struct
{	const char * name ;
	const char * artist ;
	const char * create_date ;
} TEMP_INFO ;



static void usage_exit (const char *progname, int exit_code) ;
static void missing_param (const char * option) ;
static void read_localtime (struct tm * timedata) ;

static void apply_changes (const char * filenames [2], const SF_BROADCAST_INFO * binfo, int coding_hist_append, const TEMP_INFO * info) ;

static void merge_broadcast_info (SF_BROADCAST_INFO * binfo, const SF_BROADCAST_INFO * new_binfo, int coding_hist_append) ;
static void update_strings (SNDFILE * file, const TEMP_INFO * new_tinfo) ;



int
main (int argc, char *argv [])
{	SF_BROADCAST_INFO binfo ;
	TEMP_INFO temp_info ;
	struct tm timedata ;
	const char *progname ;
	const char * filenames [2] = { NULL, NULL } ;
	int	k, coding_hist_append = 0 ;

	/* Store the program name. */
	progname = strrchr (argv [0], '/') ;
	progname = progname ? progname + 1 : argv [0] ;

	/* Check if we've been asked for help. */
	if (argc < 3 || strcmp (argv [1], "--help") == 0 || strcmp (argv [1], "-h") == 0)
		usage_exit (progname, 0) ;

	/* Clear set all fields of the struct to zero bytes. */
	memset (&binfo, 0, sizeof (binfo)) ;
	memset (&temp_info, 0, sizeof (temp_info)) ;

	/* Get the time in case we need it later. */
	read_localtime (&timedata) ;

	for (k = 1 ; k < argc ; k++)
	{	if (argv [k][0] != '-')
		{	if (filenames [0] == NULL)
				filenames [0] = argv [k] ;
			else if (filenames [1] == NULL)
				filenames [1] = argv [k] ;
			else
			{	printf ("Error : Already have two file names on the command line and then found '%s'.\n\n", argv [k]) ;
				usage_exit (progname, 1) ;
				} ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-description") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			strncpy (binfo.description, argv [k], sizeof (binfo.description)) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-originator") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			strncpy (binfo.originator, argv [k], sizeof (binfo.originator)) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-orig-ref") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			strncpy (binfo.originator_reference, argv [k], sizeof (binfo.originator_reference)) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-umid") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			strncpy (binfo.umid, argv [k], sizeof (binfo.umid)) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-orig-date") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			strncpy (binfo.origination_date, argv [k], sizeof (binfo.origination_date)) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-orig-time") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			strncpy (binfo.origination_time, argv [k], sizeof (binfo.origination_time)) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-coding-hist") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			strncpy (binfo.coding_history, argv [k], sizeof (binfo.coding_history)) ;
			binfo.coding_history_size = sizeof (binfo.coding_history) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-coding-hist-append") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			strncpy (binfo.coding_history, argv [k], sizeof (binfo.coding_history)) ;
			binfo.coding_history_size = sizeof (binfo.coding_history) ;

			coding_hist_append = 1 ;
			continue ;
			} ;

		if (strcmp (argv [k], "--str-name") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			temp_info.name = argv [k] ;
			continue ;
			} ;

		if (strcmp (argv [k], "--str-artist") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			temp_info.artist = argv [k] ;
			continue ;
			} ;

		if (strcmp (argv [k], "--str-create-date") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			temp_info.create_date = argv [k] ;
			continue ;
			} ;

		/* Following options do not take an argument. */
		if (strcmp (argv [k], "--bext-auto-time-date") == 0)
		{	char tmp [20] ;
			snprintf (tmp, sizeof (tmp), "%02d:%02d:%02d", timedata.tm_hour, timedata.tm_min, timedata.tm_sec) ;
			strncpy (binfo.origination_time, tmp, sizeof (binfo.origination_time)) ;

			snprintf (tmp, sizeof (tmp), "%04d-%02d-%02d", timedata.tm_year + 1900, timedata.tm_mon + 1, timedata.tm_mday) ;
			strncpy (binfo.origination_date, tmp, sizeof (binfo.origination_date)) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-auto-time") == 0)
		{	char tmp [20] ;
			snprintf (tmp, sizeof (tmp), "%02d:%02d:%02d", timedata.tm_hour, timedata.tm_min, timedata.tm_sec) ;
			strncpy (binfo.origination_time, tmp, sizeof (binfo.origination_time)) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-auto-date") == 0)
		{	char tmp [20] ;
			snprintf (tmp, sizeof (tmp), "%04d-%02d-%02d", timedata.tm_year + 1900, timedata.tm_mon + 1, timedata.tm_mday) ;
			strncpy (binfo.origination_date, tmp, sizeof (binfo.origination_date)) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--str-auto-create-date") == 0)
		{	char tmp [20] ;

			snprintf (tmp, sizeof (tmp), "%04d-%02d-%02d", timedata.tm_year + 1900, timedata.tm_mon + 1, timedata.tm_mday) ;

			temp_info.create_date = strdup (tmp) ;
			continue ;
			} ;

		printf ("Error : Don't know what to do with command line arg '%s'.\n\n", argv [k]) ;
		usage_exit (progname, 1) ;
		} ;

	if (filenames [0] == NULL)
	{	printf ("Error : No input file specificed.\n\n") ;
		exit (1) ;
		} ;

	if (filenames [1] != NULL && strcmp (filenames [0], filenames [1]) == 0)
	{	printf ("Error : Input and output files are the same.\n\n") ;
		exit (1) ;
		} ;

	apply_changes (filenames, &binfo, coding_hist_append, &temp_info) ;

	return 0 ;
} /* main */

/*==============================================================================
**	Print version and usage.
*/

static void
usage_exit (const char *progname, int exit_code)
{	printf ("Usage :\n  %s <file> ...\n", progname) ;
	printf ("    Fill in more later.\n\n") ;
	exit (exit_code) ;
} /* usage_exit */

static void
missing_param (const char * option)
{
	printf ("Error : Option '%s' needs a parameter but doesn't seem to have one.\n\n", option) ;
	exit (1) ;
} /* missing_param */

/*==============================================================================
*/

static void
read_localtime (struct tm * timedata)
{	time_t		current ;
	struct tm	*tmptr ;

	time (&current) ;
	memset (timedata, 0, sizeof (struct tm)) ;

#if defined (HAVE_LOCALTIME_R)
	/* If the re-entrant version is available, use it. */
	tmptr = localtime_r (&current, timedata) ;
#elif defined (HAVE_LOCALTIME)
	/* Otherwise use the standard one and copy the data to local storage. */
	tmptr = localtime (&current) ;
	memcpy (timedata, tmptr, sizeof (struct tm)) ;
#endif

	return ;
} /* read_localtime */

static void
apply_changes (const char * filenames [2], const SF_BROADCAST_INFO * new_binfo, int coding_hist_append, const TEMP_INFO * new_tinfo)
{	SNDFILE *infile = NULL, *outfile = NULL ;
	SF_INFO sfinfo ;
	SF_BROADCAST_INFO binfo ;
	TEMP_INFO tinfo ;
	int error_code = 0, infileminor ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	memset (&tinfo, 0, sizeof (tinfo)) ;

	if (filenames [1] == NULL)
		infile = outfile = sf_open (filenames [0], SFM_RDWR, &sfinfo) ;
	else
	{	infile = sf_open (filenames [0], SFM_READ, &sfinfo) ;

		/* Output must be WAV. */
		sfinfo.format = SF_FORMAT_WAV | (SF_FORMAT_SUBMASK & sfinfo.format) ;
		outfile = sf_open (filenames [1], SFM_WRITE, &sfinfo) ;
		} ;

	if (infile == NULL)
	{	printf ("Error : Not able to open input file '%s' : %s\n", filenames [0], sf_strerror (infile)) ;
		error_code = 1 ;
		goto cleanup_exit ;
		} ;

	if (outfile == NULL)
	{	printf ("Error : Not able to open output file '%s' : %s\n", filenames [1], sf_strerror (outfile)) ;
		error_code = 1 ;
		goto cleanup_exit ;
		} ;

	if ((SF_FORMAT_TYPEMASK & sfinfo.format) != SF_FORMAT_WAV)
	{	printf ("Error : This is not a WAV file and hence broadcast info cannot be added to it.\n\n") ;
		error_code = 1 ;
		goto cleanup_exit ;
		} ;

	infileminor = SF_FORMAT_SUBMASK & sfinfo.format ;

	switch (infileminor)
	{	case SF_FORMAT_PCM_16 :
		case SF_FORMAT_PCM_24 :
		case SF_FORMAT_PCM_32 :
			break ;

		default :
			printf (
				"Warning : The EBU Technical Recommendation R68-2000 states that the only\n"
				"          allowed encodings are Linear PCM and MPEG3. This file is not in\n"
				"          the right format.\n\n"
				) ;
			break ;
		} ;

	if (sf_command (infile, SFC_GET_BROADCAST_INFO, &binfo, sizeof (binfo)) == 0)
	{	if (infile == outfile)
		{	printf (
				"Error : Attempting in-place broadcast info update, but file does not\n"
				"        have a 'bext' chunk to modify. The solution is to specify both\n"
				"        input and output files on the command line.\n\n"
				) ;
			error_code = 1 ;
			goto cleanup_exit ;
			} ;

		memset (&binfo, 0, sizeof (binfo)) ;
		} ;

	/* Merge and write broadcast info. */
	merge_broadcast_info (&binfo, new_binfo, coding_hist_append) ;

	if (sf_command (outfile, SFC_SET_BROADCAST_INFO, &binfo, sizeof (binfo)) == 0)
		printf ("Error : Setting of broadcast info chunks failed.\n\n") ;

	/* Grab the strings from the existing file. */
	tinfo.name = sf_get_string (infile, SF_STR_TITLE) ;
	tinfo.artist = sf_get_string (infile, SF_STR_ARTIST) ;
	tinfo.create_date = sf_get_string (infile, SF_STR_DATE) ;

	/* If the input and output files are different we merge the strings and
	**	then clear the old versions.
	*/
	tinfo.name = new_tinfo->name ? new_tinfo->name : tinfo.name ;
	tinfo.artist = new_tinfo->artist ? new_tinfo->artist : tinfo.artist ;
	tinfo.create_date = new_tinfo->create_date ? new_tinfo->create_date : tinfo.create_date ;

	update_strings (outfile, &tinfo) ;

	if (infile != outfile)
	{	/* If the input file is not the same as the output file, copy the data. */
		if ((infileminor == SF_FORMAT_DOUBLE) || (infileminor == SF_FORMAT_FLOAT))
			sfe_copy_data_fp (outfile, infile, sfinfo.channels) ;
		else
			sfe_copy_data_int (outfile, infile, sfinfo.channels) ;
		} ;

cleanup_exit :

	if (outfile != NULL && outfile != infile)
		sf_close (outfile) ;

	if (infile != NULL)
		sf_close (infile) ;

	if (error_code)
		exit (error_code) ;

	return ;
} /* apply_changes */

static void
merge_broadcast_info (SF_BROADCAST_INFO * binfo, const SF_BROADCAST_INFO * new_binfo, int coding_hist_append)
{
#define REPLACE_IF_NEW(x) \
		if (strlen (new_binfo->x) > 0) \
			memcpy (binfo->x, new_binfo->x, sizeof (binfo->x)) ;


	REPLACE_IF_NEW (description) ;
	REPLACE_IF_NEW (originator) ;
	REPLACE_IF_NEW (originator_reference) ;
	REPLACE_IF_NEW (origination_date) ;
	REPLACE_IF_NEW (origination_time) ;
	REPLACE_IF_NEW (umid) ;

	/* Special case for coding_history because we may want to append. */
	if (strlen (new_binfo->coding_history) > 0)
	{	if (coding_hist_append)
		{	int slen = strlen (binfo->coding_history) ;

			while (slen > 1 && isspace (binfo->coding_history [slen - 1]))
				slen -- ;

			binfo->coding_history [slen++] = '\n' ;
			memcpy (binfo->coding_history + slen, new_binfo->coding_history, sizeof (binfo->coding_history) - slen) ;
			}
		else
		{	memcpy (binfo->coding_history, new_binfo->coding_history, sizeof (binfo->coding_history)) ;
			binfo->coding_history_size = sizeof (binfo->coding_history) ;
			} ;
		} ;

} /* merge_broadcast_info*/

static void
update_strings (SNDFILE * file, const TEMP_INFO * new_tinfo)
{
	if (new_tinfo->name != NULL)
		sf_set_string (file, SF_STR_TITLE, new_tinfo->name) ;

	if (new_tinfo->artist != NULL)
		sf_set_string (file, SF_STR_ARTIST, new_tinfo->artist) ;

	if (new_tinfo->create_date != NULL)
		sf_set_string (file, SF_STR_DATE, new_tinfo->create_date) ;

} /* update_strings */

