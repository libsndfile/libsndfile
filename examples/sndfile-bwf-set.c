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

#define	BUFFER_LEN		(1 << 16)


#define	SAFE_STRNCPY(dest,src) \
			{ strncpy (dest, src, sizeof (dest)) ; dest [sizeof (dest) - 1] = 0 ; }

static void usage_exit (const char *progname, int exit_code) ;
static void missing_param (const char * option) ;
static void read_time (struct tm * timedata) ;

static void broadcast_dump (const SF_BROADCAST_INFO * bext) ;


typedef struct
{	const char * name ;
	const char * artist ;
	const char * create_date ;
} TEMP_INFO ;


int
main (int argc, char *argv [])
{	SF_BROADCAST_INFO new_binfo ;
	TEMP_INFO temp_info ;
	struct tm timedata ;
	const char *progname ;
	const char * filename = NULL ;
	int	k ;

	/* Store the program name. */
	progname = strrchr (argv [0], '/') ;
	progname = progname ? progname + 1 : argv [0] ;

	/* Check if we've been asked for help. */
	if (argc < 2 || strcmp (argv [1], "--help") == 0 || strcmp (argv [1], "-h") == 0)
		usage_exit (progname, 0) ;

	/* Clear set all fields of the struct to zero bytes. */
	memset (&new_binfo, 0, sizeof (new_binfo)) ;
	memset (&temp_info, 0, sizeof (temp_info)) ;

	/* Get the time in case we need it later. */
	read_time (&timedata) ;

	for (k = 1 ; k < argc ; k++)
	{	if (argv [k][0] != '-')
		{	if (filename != NULL)
			{	printf ("Error : have filename '%s' and just found another '%s'.\n\n", filename, argv [k]) ;
				usage_exit (progname, 1) ;
				} ;
			filename = argv [k] ;
			continue ;
			} ;

		if (strcmp (argv [k], "--description") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			SAFE_STRNCPY (new_binfo.description, argv [k]) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--originator") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			SAFE_STRNCPY (new_binfo.originator, argv [k]) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--orig-ref") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			SAFE_STRNCPY (new_binfo.originator_reference, argv [k]) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--umid") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			SAFE_STRNCPY (new_binfo.umid, argv [k]) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--orig-date") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			SAFE_STRNCPY (new_binfo.origination_date, argv [k]) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--orig-time") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			SAFE_STRNCPY (new_binfo.origination_time, argv [k]) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--coding-hist-append") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			SAFE_STRNCPY (new_binfo.coding_history, argv [k]) ;
			new_binfo.coding_history_size = sizeof (new_binfo.coding_history) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--info-name") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			temp_info.name = argv [k] ;
			continue ;
			} ;

		if (strcmp (argv [k], "--info-artist") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			temp_info.artist = argv [k] ;
			continue ;
			} ;

		if (strcmp (argv [k], "--info-cr-date") == 0)
		{	k ++ ;
			if (k == argc) missing_param (argv [k - 1]) ;

			temp_info.create_date = argv [k] ;
			continue ;
			} ;

		/* Following options do not take an argument. */
		if (strcmp (argv [k], "--auto-time-date") == 0)
		{	char tmp [20] ;
			snprintf (tmp, sizeof (tmp), "%02d-%02d-%02d", timedata.tm_hour, timedata.tm_min, timedata.tm_sec) ;
			strncpy (new_binfo.origination_time, tmp, sizeof (new_binfo.origination_time)) ;

			snprintf (tmp, sizeof (tmp), "%04d-%02d-%02d", timedata.tm_year + 1900, timedata.tm_mon, timedata.tm_mday) ;
			strncpy (new_binfo.origination_date, tmp, sizeof (new_binfo.origination_date)) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--auto-time") == 0)
		{	char tmp [20] ;
			snprintf (tmp, sizeof (tmp), "%02d-%02d-%02d", timedata.tm_hour, timedata.tm_min, timedata.tm_sec) ;
			strncpy (new_binfo.origination_time, tmp, sizeof (new_binfo.origination_time)) ;
			continue ;
			} ;

		if (strcmp (argv [k], "--auto-date") == 0)
		{	char tmp [20] ;
			snprintf (tmp, sizeof (tmp), "%04d-%02d-%02d", timedata.tm_year + 1900, timedata.tm_mon, timedata.tm_mday) ;
			strncpy (new_binfo.origination_date, tmp, sizeof (new_binfo.origination_date)) ;
			continue ;
			} ;

		printf ("Error : Don't know what to do with command line arg '%s'.\n\n", argv [k]) ;
		usage_exit (progname, 1) ;
		} ;

	broadcast_dump (&new_binfo) ;
	if (temp_info.name)
		printf ("Info Name : %s\n", temp_info.name) ;
	if (temp_info.artist)
		printf ("Info Artist : %s\n", temp_info.artist) ;
	if (temp_info.create_date)
		printf ("Info Date : %s\n", temp_info.create_date) ;

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
read_time (struct tm * timedata)
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
} /* read_time */

static void
broadcast_dump (const SF_BROADCAST_INFO * bext)
{	/*
	**	From : http://www.ebu.ch/en/technical/publications/userguides/bwf_user_guide.php
	**
	**	Time Reference:
	**		This field is a count from midnight in samples to the first sample
	**		of the audio sequence.
	*/

	printf ("Description      : %.*s\n", (int) sizeof (bext->description), bext->description) ;
	printf ("Originator       : %.*s\n", (int) sizeof (bext->originator), bext->originator) ;
	printf ("Origination ref  : %.*s\n", (int) sizeof (bext->originator_reference), bext->originator_reference) ;
	printf ("Origination date : %.*s\n", (int) sizeof (bext->origination_date), bext->origination_date) ;
	printf ("Origination time : %.*s\n", (int) sizeof (bext->origination_time), bext->origination_time) ;

	printf ("Time ref         : 0\n") ;

	printf ("BWF version      : %d\n", bext->version) ;
	printf ("UMID             : %.*s\n", (int) sizeof (bext->umid), bext->umid) ;
	printf ("Coding history   : %.*s\n", bext->coding_history_size, bext->coding_history) ;

} /* broadcast_dump */

