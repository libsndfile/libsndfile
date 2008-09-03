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

	/* Stuff to go in the 'bext' chunk of WAV files. */
	int has_bext_fields ;
	int coding_hist_append ;

	const char * description ;
	const char * originator ;
	const char * originator_reference ;
	const char * origination_date ;
	const char * origination_time ;
	const char * umid ;
	const char * coding_history ;
} INFO ;



static void usage_exit (const char *progname, int exit_code) ;
static void missing_param (const char * option) ;
static void read_localtime (struct tm * timedata) ;

static void apply_changes (const char * filenames [2], const INFO * info) ;

static int merge_broadcast_info (SNDFILE * infile, SNDFILE * outfile, int format, const INFO * info) ;
static void update_strings (SNDFILE * outfile, const INFO * info) ;

static int has_bext_fields_set (const INFO * info) ;

int
main (int argc, char *argv [])
{	INFO info ;
	struct tm timedata ;
	const char *progname ;
	const char * filenames [2] = { NULL, NULL } ;
	int	k ;

	/* Store the program name. */
	progname = strrchr (argv [0], '/') ;
	progname = progname ? progname + 1 : argv [0] ;

	/* Check if we've been asked for help. */
	if (argc < 3 || strcmp (argv [1], "--help") == 0 || strcmp (argv [1], "-h") == 0)
		usage_exit (progname, 0) ;

	/* Clear set all fields of the struct to zero bytes. */
	memset (&info, 0, sizeof (info)) ;

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

			info.description = argv [k] ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-originator") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			info.originator = argv [k] ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-orig-ref") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			info.originator_reference = argv [k] ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-umid") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			info.umid = argv [k] ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-orig-date") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			info.origination_date = argv [k] ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-orig-time") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			info.origination_time = argv [k] ;
			puts (info.origination_time) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-coding-hist") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			info.coding_history = argv [k] ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-coding-hist-append") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			info.coding_history = argv [k] ;
			continue ;
			} ;

		if (strcmp (argv [k], "--str-name") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			info.name = argv [k] ;
			continue ;
			} ;

		if (strcmp (argv [k], "--str-artist") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			info.artist = argv [k] ;
			continue ;
			} ;

		if (strcmp (argv [k], "--str-create-date") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			info.create_date = argv [k] ;
			continue ;
			} ;

		/* Following options do not take an argument. */
		if (strcmp (argv [k], "--bext-auto-time-date") == 0)
		{	char tmp [20] ;
			snprintf (tmp, sizeof (tmp), "%02d:%02d:%02d", timedata.tm_hour, timedata.tm_min, timedata.tm_sec) ;
			info.origination_time = strdup (tmp) ;

			snprintf (tmp, sizeof (tmp), "%04d-%02d-%02d", timedata.tm_year + 1900, timedata.tm_mon + 1, timedata.tm_mday) ;
			info.origination_date = strdup (tmp) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-auto-time") == 0)
		{	char tmp [20] ;
			snprintf (tmp, sizeof (tmp), "%02d:%02d:%02d", timedata.tm_hour, timedata.tm_min, timedata.tm_sec) ;
			info.origination_time = strdup (tmp) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--bext-auto-date") == 0)
		{	char tmp [20] ;
			snprintf (tmp, sizeof (tmp), "%04d-%02d-%02d", timedata.tm_year + 1900, timedata.tm_mon + 1, timedata.tm_mday) ;
			info.origination_date = strdup (tmp) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--str-auto-create-date") == 0)
		{	char tmp [20] ;

			snprintf (tmp, sizeof (tmp), "%04d-%02d-%02d", timedata.tm_year + 1900, timedata.tm_mon + 1, timedata.tm_mday) ;

			info.create_date = strdup (tmp) ;
			continue ;
			} ;

		printf ("Error : Don't know what to do with command line arg '%s'.\n\n", argv [k]) ;
		usage_exit (progname, 1) ;
		} ;

	/* Find out if any of the 'bext' fields are set. */
	info.has_bext_fields = has_bext_fields_set (&info) ;

	if (filenames [0] == NULL)
	{	printf ("Error : No input file specificed.\n\n") ;
		exit (1) ;
		} ;

	if (filenames [1] != NULL && strcmp (filenames [0], filenames [1]) == 0)
	{	printf ("Error : Input and output files are the same.\n\n") ;
		exit (1) ;
		} ;

	apply_changes (filenames, &info) ;

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

static int
has_bext_fields_set (const INFO * info)
{
	if (info->description || info->originator || info->originator_reference)
		return 1 ;
		
	if (info->origination_date || info->origination_time || info->umid || info->coding_history)
		return 1 ;
	
	return 0 ;
} /* has_bext_fields_set */

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
apply_changes (const char * filenames [2], const INFO * info)
{	SNDFILE *infile = NULL, *outfile = NULL ;
	SF_INFO sfinfo ;
	INFO tmpinfo ;
	int error_code = 0 ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	memset (&tmpinfo, 0, sizeof (tmpinfo)) ;

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

	if (info->has_bext_fields && merge_broadcast_info (infile, outfile, sfinfo.format, info))
	{	error_code = 1 ;
		goto cleanup_exit ;
		} ;

	update_strings (outfile, info) ;

	if (infile != outfile)
	{	int infileminor = SF_FORMAT_SUBMASK & sfinfo.format ;

		/* If the input file is not the same as the output file, copy the data. */
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

static int
merge_broadcast_info (SNDFILE * infile, SNDFILE * outfile, int format, const INFO * info)
{	SF_BROADCAST_INFO binfo ;
	int infileminor ;

	memset (&binfo, 0, sizeof (binfo)) ;

	if ((SF_FORMAT_TYPEMASK & format) != SF_FORMAT_WAV)
	{	printf ("Error : This is not a WAV file and hence broadcast info cannot be added to it.\n\n") ;
		return 1 ;
		} ;

	infileminor = SF_FORMAT_SUBMASK & format ;

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
			return 1 ;
			} ;
		} ;

#define REPLACE_IF_NEW(x) \
		if (info->x != NULL) \
			memcpy (binfo.x, info->x, sizeof (binfo.x)) ;

	REPLACE_IF_NEW (description) ;
	REPLACE_IF_NEW (originator) ;
	REPLACE_IF_NEW (originator_reference) ;
	REPLACE_IF_NEW (origination_date) ;
	REPLACE_IF_NEW (origination_time) ;
	REPLACE_IF_NEW (umid) ;

	/* Special case for coding_history because we may want to append. */
	if (info->coding_history != NULL)
	{	if (info->coding_hist_append)
		{	int slen = strlen (binfo.coding_history) ;

			while (slen > 1 && isspace (binfo.coding_history [slen - 1]))
				slen -- ;

			binfo.coding_history [slen++] = '\n' ;
			memcpy (binfo.coding_history + slen, info->coding_history, sizeof (binfo.coding_history) - slen) ;
			}
		else
		{	memcpy (binfo.coding_history, info->coding_history, sizeof (binfo.coding_history)) ;
			binfo.coding_history_size = sizeof (binfo.coding_history) ;
			} ;
		} ;

	if (sf_command (outfile, SFC_SET_BROADCAST_INFO, &binfo, sizeof (binfo)) == 0)
		printf ("Error : Setting of broadcast info chunks failed.\n\n") ;

	return 0 ;
} /* merge_broadcast_info*/

static void
update_strings (SNDFILE * outfile, const INFO * info)
{
	if (info->name != NULL)
		sf_set_string (outfile, SF_STR_TITLE, info->name) ;

	if (info->artist != NULL)
		sf_set_string (outfile, SF_STR_ARTIST, info->artist) ;

	if (info->create_date != NULL)
		sf_set_string (outfile, SF_STR_DATE, info->create_date) ;

} /* update_strings */

