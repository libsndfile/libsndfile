/*
** Copyright (C) 2002-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (C) 2003 Ross Bencina <rbencina@iprimus.com.au>
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

/*
** This header file MUST be included before the others to ensure that
** large file support is enabled.
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if (HAVE_DECL_S_IRGRP == 0)
#include <sf_unistd.h>
#endif

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#if (defined (__MWERKS__) && defined (macintosh))
typedef int ssize_t ;
#include <Files.h>
#endif

#include "sndfile.h"
#include "common.h"

#define	SENSIBLE_SIZE	(0x40000000)

static int psf_open_fd (const char * path, int mode) ;
static int psf_close_fd (int fd) ;
static void psf_log_syserr (SF_PRIVATE *psf, int error) ;
static sf_count_t psf_get_filelen_fd (int fd) ;

int
psf_fopen (SF_PRIVATE *psf, const char *pathname, int open_mode)
{
	psf->error = 0 ;
	psf->filedes = psf_open_fd (pathname, open_mode) ;

	if (psf->filedes == - SFE_BAD_OPEN_MODE)
	{	psf->error = SFE_BAD_OPEN_MODE ;
		psf->filedes = -1 ;
		return psf->error ;
		} ;

	if (psf->filedes == -1)
		psf_log_syserr (psf, errno) ;

	psf->mode = open_mode ;

	return psf->error ;
} /* psf_fopen */

int
psf_fclose (SF_PRIVATE *psf)
{	int retval ;

	if (psf->do_not_close_descriptor)
	{	psf->filedes = -1 ;
		return 0 ;
		} ;

	if ((retval = psf_close_fd (psf->filedes)) == -1)
		psf_log_syserr (psf, errno) ;

	psf->filedes = -1 ;

	return retval ;
} /* psf_fclose */

int
psf_open_rsrc (SF_PRIVATE *psf, int open_mode)
{	char *fname ;

	if (psf->rsrcdes > 0)
		return 0 ;

	/* Test for MacOSX style resource fork on HPFS or HPFS+ filesystems. */
	LSF_SNPRINTF (psf->rsrcpath, sizeof (psf->rsrcpath), "%s/rsrc", psf->filepath) ;

	psf->error = SFE_NO_ERROR ;
	if ((psf->rsrcdes = psf_open_fd (psf->rsrcpath, open_mode)) >= 0)
	{	psf->rsrclength = psf_get_filelen_fd (psf->rsrcdes) ;
		if (psf->rsrclength < 100)
		{	psf->error = SFE_SD2_BAD_RSRC ;
			return psf->error ;
			} ;
		return SFE_NO_ERROR ;
		} ;

	if (psf->rsrcdes == - SFE_BAD_OPEN_MODE)
	{	psf->error = SFE_BAD_OPEN_MODE ;
		return psf->error ;
		} ;

	/*
	** Now try for a resource fork stored as a separate file.
	** Grab the un-adulterated filename again.
	*/
	LSF_SNPRINTF (psf->rsrcpath, sizeof (psf->rsrcpath), "%s", psf->filepath) ;

	if ((fname = strrchr (psf->rsrcpath, '/')) != NULL)
		fname ++ ;
	else if ((fname = strrchr (psf->rsrcpath, '\\')) != NULL)
		fname ++ ;
	else
		fname = psf->rsrcpath ;

	memmove (fname + 2, fname, strlen (fname) + 1) ;
	fname [0] = '.' ;
	fname [1] = '_' ;

	psf->error = SFE_NO_ERROR ;
	if ((psf->rsrcdes = psf_open_fd (psf->rsrcpath, open_mode)) >= 0)
	{	psf->rsrclength = psf_get_filelen_fd (psf->rsrcdes) ;
		return SFE_NO_ERROR ;
		} ;

	if (psf->rsrcdes == -1)
		psf_log_syserr (psf, errno) ;

	psf->rsrcdes = -1 ;

	return psf->error ;
} /* psf_open_rsrc */

int
psf_close_rsrc (SF_PRIVATE *psf)
{
	psf_close_fd (psf->rsrcdes) ;
	psf->rsrcdes = -1 ;
	return 0 ;
} /* psf_close_rsrc */

sf_count_t
psf_get_filelen (SF_PRIVATE *psf)
{	sf_count_t	filelen ;

	filelen = psf_get_filelen_fd (psf->filedes) ;

	if (filelen == -1)
	{	psf_log_syserr (psf, errno) ;
		return (sf_count_t) -1 ;
		} ;

	if (filelen == -SFE_BAD_STAT_SIZE)
	{	psf->error = SFE_BAD_STAT_SIZE ;
		return (sf_count_t) -1 ;
		} ;

	switch (psf->mode)
	{	case SFM_WRITE :
			filelen = filelen - psf->fileoffset ;
			break ;

		case SFM_READ :
			if (psf->fileoffset > 0 && psf->filelength > 0)
				filelen = psf->filelength ;
			break ;

		case SFM_RDWR :
			/*
			** Cannot open embedded files SFM_RDWR so we don't need to
			** subtract psf->fileoffset. We already have the answer we
			** need.
			*/
			break ;

		default :
			/* Shouldn't be here, so return error. */
			filelen = -1 ;
		} ;

	return filelen ;
} /* psf_get_filelen */

#if ((defined (WIN32) || defined (_WIN32)) == 0)

/*------------------------------------------------------------------------------
** Win32 stuff at the bottom of the file. Unix and other sensible OSes here.
*/

int
psf_set_stdio (SF_PRIVATE *psf, int mode)
{	int	error = 0 ;

	switch (mode)
	{	case SFM_RDWR :
				error = SFE_OPEN_PIPE_RDWR ;
				break ;

		case SFM_READ :
				psf->filedes = 0 ;
				break ;

		case SFM_WRITE :
				psf->filedes = 1 ;
				break ;

		default :
				error = SFE_BAD_OPEN_MODE ;
				break ;
		} ;
	psf->filelength = 0 ;

	return error ;
} /* psf_set_stdio */

void
psf_set_file (SF_PRIVATE *psf, int fd)
{	psf->filedes = fd ;
} /* psf_set_file */

int
psf_filedes_valid (SF_PRIVATE *psf)
{	return (psf->filedes >= 0) ? SF_TRUE : SF_FALSE ;
} /* psf_set_file */

sf_count_t
psf_fseek (SF_PRIVATE *psf, sf_count_t offset, int whence)
{	sf_count_t	new_position ;

	switch (whence)
	{	case SEEK_SET :
				offset += psf->fileoffset ;
				break ;

		case SEEK_END :
				if (psf->mode == SFM_WRITE)
				{	new_position = lseek (psf->filedes, offset, whence) ;

					if (new_position < 0)
						psf_log_syserr (psf, errno) ;

					return new_position - psf->fileoffset ;
					} ;

				/* Transform SEEK_END into a SEEK_SET, ie find the file
				** length add the requested offset (should be <= 0) to
				** get the offset wrt the start of file.
				*/
				whence = SEEK_SET ;
				offset = lseek (psf->filedes, 0, SEEK_END) + offset ;
				break ;

		default :
				/* No need to do anything about SEEK_CUR. */
				break ;
		} ;

	new_position = lseek (psf->filedes, offset, whence) ;

	if (new_position < 0)
		psf_log_syserr (psf, errno) ;

	new_position -= psf->fileoffset ;

	return new_position ;
} /* psf_fseek */

sf_count_t
psf_fread (void *ptr, sf_count_t bytes, sf_count_t items, SF_PRIVATE *psf)
{	sf_count_t total = 0 ;
	ssize_t	count ;

	items *= bytes ;

	/* Do this check after the multiplication above. */
	if (items <= 0)
		return 0 ;

	while (items > 0)
	{	/* Break the writes down to a sensible size. */
		count = (items > SENSIBLE_SIZE) ? SENSIBLE_SIZE : (ssize_t) items ;

		count = read (psf->filedes, ((char*) ptr) + total, (size_t) count) ;

		if (count == -1)
		{	if (errno == EINTR)
				continue ;

			psf_log_syserr (psf, errno) ;
			break ;
			} ;

		if (count == 0)
			break ;

		total += count ;
		items -= count ;
		} ;

	if (psf->is_pipe)
		psf->pipeoffset += total ;

	return total / bytes ;
} /* psf_fread */

sf_count_t
psf_fwrite (const void *ptr, sf_count_t bytes, sf_count_t items, SF_PRIVATE *psf)
{	sf_count_t total = 0 ;
	ssize_t	count ;

	items *= bytes ;

	/* Do this check after the multiplication above. */
	if (items <= 0)
		return 0 ;

	while (items > 0)
	{	/* Break the writes down to a sensible size. */
		count = (items > SENSIBLE_SIZE) ? SENSIBLE_SIZE : items ;

		count = write (psf->filedes, ((const char*) ptr) + total, count) ;

		if (count == -1)
		{	if (errno == EINTR)
				continue ;

			psf_log_syserr (psf, errno) ;
			break ;
			} ;

		if (count == 0)
			break ;

		total += count ;
		items -= count ;
		} ;

	if (psf->is_pipe)
		psf->pipeoffset += total ;

	return total / bytes ;
} /* psf_fwrite */

sf_count_t
psf_ftell (SF_PRIVATE *psf)
{	sf_count_t pos ;

	if (psf->is_pipe)
		return psf->pipeoffset ;

	pos = lseek (psf->filedes, 0, SEEK_CUR) ;

	if (pos == ((sf_count_t) -1))
	{	psf_log_syserr (psf, errno) ;
		return -1 ;
		} ;

	return pos - psf->fileoffset ;
} /* psf_ftell */

int
psf_close_fd (int fd)
{	int retval ;

	while ((retval = close (fd)) == -1 && errno == EINTR)
		/* Do nothing. */ ;

	return retval ;
} /* psf_close_fd */

sf_count_t
psf_fgets (char *buffer, sf_count_t bufsize, SF_PRIVATE *psf)
{	sf_count_t	k = 0 ;
	sf_count_t		count ;

	while (k < bufsize - 1)
	{	count = read (psf->filedes, &(buffer [k]), 1) ;

		if (count == -1)
		{	if (errno == EINTR)
				continue ;

			psf_log_syserr (psf, errno) ;
			break ;
			} ;

		if (count == 0 || buffer [k++] == '\n')
			break ;
		} ;

	buffer [k] = 0 ;

	return k ;
} /* psf_fgets */

int
psf_is_pipe (SF_PRIVATE *psf)
{	struct stat statbuf ;

	if (fstat (psf->filedes, &statbuf) == -1)
	{	psf_log_syserr (psf, errno) ;
		/* Default to maximum safety. */
		return SF_TRUE ;
		} ;

	if (S_ISFIFO (statbuf.st_mode) || S_ISSOCK (statbuf.st_mode))
		return SF_TRUE ;

	return SF_FALSE ;
} /* psf_is_pipe */

static sf_count_t
psf_get_filelen_fd (int fd)
{	struct stat statbuf ;

	/*
	** Sanity check.
	** If everything is OK, this will be optimised out.
	*/
	if (sizeof (statbuf.st_size) == 4 && sizeof (sf_count_t) == 8)
		return (sf_count_t) -SFE_BAD_STAT_SIZE ;

/* Cygwin seems to need this. */
#if (defined (__CYGWIN__) && HAVE_FSYNC)
	fsync (psf->filedes) ;
#endif

	if (fstat (fd, &statbuf) == -1)
		return (sf_count_t) -1 ;

	return statbuf.st_size ;
} /* psf_get_filelen_fd */

int
psf_ftruncate (SF_PRIVATE *psf, sf_count_t len)
{	int retval ;

	/* Returns 0 on success, non-zero on failure. */
	if (len < 0)
		return -1 ;

	if ((sizeof (off_t) < sizeof (sf_count_t)) && len > 0x7FFFFFFF)
		return -1 ;

#if (defined (__MWERKS__) && defined (macintosh))
	retval = FSSetForkSize (psf->filedes, fsFromStart, len) ;
#else
	retval = ftruncate (psf->filedes, len) ;
#endif

	if (retval == -1)
		psf_log_syserr (psf, errno) ;

	return retval ;
} /* psf_ftruncate */

static int
psf_open_fd (const char * pathname, int open_mode)
{	int fd, oflag, mode ;

	/*
	** Sanity check. If everything is OK, this test and the printfs will
	** be optimised out. This is meant to catch the problems caused by
	** "config.h" being included after <stdio.h>.
	*/
	if (sizeof (off_t) != sizeof (sf_count_t))
	{	puts ("\n\n*** Fatal error : sizeof (off_t) != sizeof (sf_count_t)") ;
		puts ("*** This means that libsndfile was not configured correctly.\n") ;
		exit (1) ;
		} ;

	switch (open_mode)
	{	case SFM_READ :
				oflag = O_RDONLY ;
				mode = 0 ;
				break ;

		case SFM_WRITE :
				oflag = O_WRONLY | O_CREAT | O_TRUNC ;
				mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ;
				break ;

		case SFM_RDWR :
				oflag = O_RDWR | O_CREAT ;
				mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ;
				break ;

		default :
				return - SFE_BAD_OPEN_MODE ;
				break ;
		} ;

#if defined (__CYGWIN__)
	oflag |= O_BINARY ;
#endif

	if (mode == 0)
		fd = open (pathname, oflag) ;
	else
		fd = open (pathname, oflag, mode) ;

	return fd ;
} /* psf_open_fd */

static void
psf_log_syserr (SF_PRIVATE *psf, int error)
{
	/* Only log an error if no error has been set yet. */
	if (psf->error == 0)
	{	psf->error = SFE_SYSTEM ;
		LSF_SNPRINTF (psf->syserr, sizeof (psf->syserr), "System error : %s.", strerror (error)) ;
		} ;

	return ;
} /* psf_log_syserr */

#elif	OS_IS_WIN32

/* Win32 file i/o functions implemented using native Win32 API */

#include <windows.h>
#include <io.h>

#ifndef HAVE_SSIZE_T
typedef long ssize_t ;
#endif

/* Win32 */ static int
psf_open_fd (const char * pathname, int open_mode)
{	DWORD dwDesiredAccess ;
	DWORD dwShareMode ;
	DWORD dwCreationDistribution ;
	HANDLE handle ;

	switch (open_mode)
	{	case SFM_READ :
				dwDesiredAccess = GENERIC_READ ;
				dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE ;
				dwCreationDistribution = OPEN_EXISTING ;
				break ;

		case SFM_WRITE :
				dwDesiredAccess = GENERIC_WRITE ;
				dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE ;
				dwCreationDistribution = CREATE_ALWAYS ;
				break ;

		case SFM_RDWR :
				dwDesiredAccess = GENERIC_READ | GENERIC_WRITE ;
				dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE ;
				dwCreationDistribution = OPEN_ALWAYS ;
				break ;

		default :
				return - SFE_BAD_OPEN_MODE ;
		} ;

	handle = CreateFile (
			pathname,					/* pointer to name of the file */
			dwDesiredAccess,			/* access (read-write) mode */
			dwShareMode,				/* share mode */
			0,							/* pointer to security attributes */
			dwCreationDistribution,		/* how to create */
			FILE_ATTRIBUTE_NORMAL,		/* file attributes (could use FILE_FLAG_SEQUENTIAL_SCAN) */
			NULL						/* handle to file with attributes to copy */
			) ;

	if (handle == INVALID_HANDLE_VALUE)
		return -1 ;

	return (int) handle ;
} /* psf_open_fd */

/* Win32 */ static void
psf_log_syserr (SF_PRIVATE *psf, int error)
{	LPVOID lpMsgBuf ;

	/* Only log an error if no error has been set yet. */
	if (psf->error == 0)
	{	psf->error = SFE_SYSTEM ;

		FormatMessage (
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			error,
			MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0,
			NULL
			) ;

		LSF_SNPRINTF (psf->syserr, sizeof (psf->syserr), "System error : %s", lpMsgBuf) ;
		LocalFree (lpMsgBuf) ;
		} ;

	return ;
} /* psf_log_syserr */


/* Win32 */ int
psf_set_stdio (SF_PRIVATE *psf, int mode)
{	HANDLE	handle = NULL ;
	int	error = 0 ;

	switch (mode)
	{	case SFM_RDWR :
				error = SFE_OPEN_PIPE_RDWR ;
				break ;

		case SFM_READ :
				handle = GetStdHandle (STD_INPUT_HANDLE) ;
				psf->do_not_close_descriptor = 1 ;
				break ;

		case SFM_WRITE :
				handle = GetStdHandle (STD_OUTPUT_HANDLE) ;
				psf->do_not_close_descriptor = 1 ;
				break ;

		default :
				error = SFE_BAD_OPEN_MODE ;
				break ;
		} ;

	psf->filedes = (int) handle ;
	psf->filelength = 0 ;

	return error ;
} /* psf_set_stdio */

/* Win32 */ void
psf_set_file (SF_PRIVATE *psf, int fd)
{	HANDLE handle ;
	long osfhandle ;

	osfhandle = _get_osfhandle (fd) ;
	handle = (HANDLE) osfhandle ;

	if (GetFileType (handle) == FILE_TYPE_DISK)
		psf->filedes = (int) handle ;
	else
		psf->filedes = fd ;
} /* psf_set_file */

/* Win32 */ int
psf_filedes_valid (SF_PRIVATE *psf)
{	return (((HANDLE) psf->filedes) != INVALID_HANDLE_VALUE) ? SF_TRUE : SF_FALSE ;
} /* psf_set_file */

/* Win32 */ sf_count_t
psf_fseek (SF_PRIVATE *psf, sf_count_t offset, int whence)
{	sf_count_t new_position ;
	LONG lDistanceToMove, lDistanceToMoveHigh ;
	DWORD dwMoveMethod ;
	DWORD dwResult, dwError ;

	switch (whence)
	{	case SEEK_SET :
				offset += psf->fileoffset ;
				dwMoveMethod = FILE_BEGIN ;
				break ;

		case SEEK_END :
				dwMoveMethod = FILE_END ;
				break ;

		default :
				dwMoveMethod = FILE_CURRENT ;
				break ;
		} ;

	lDistanceToMove = (DWORD) (offset & 0xFFFFFFFF) ;
	lDistanceToMoveHigh = (DWORD) ((offset >> 32) & 0xFFFFFFFF) ;

	dwResult = SetFilePointer ((HANDLE) psf->filedes, lDistanceToMove, &lDistanceToMoveHigh, dwMoveMethod) ;

	if (dwResult == 0xFFFFFFFF)
		dwError = GetLastError () ;
	else
		dwError = NO_ERROR ;

	if (dwError != NO_ERROR)
	{	psf_log_syserr (psf, dwError) ;
		return -1 ;
		} ;

	new_position = (dwResult + ((__int64) lDistanceToMoveHigh << 32)) - psf->fileoffset ;

	return new_position ;
} /* psf_fseek */

/* Win32 */ sf_count_t
psf_fread (void *ptr, sf_count_t bytes, sf_count_t items, SF_PRIVATE *psf)
{	sf_count_t total = 0 ;
	ssize_t count ;
	DWORD dwNumberOfBytesRead ;

	items *= bytes ;

	/* Do this check after the multiplication above. */
	if (items <= 0)
		return 0 ;

	while (items > 0)
	{	/* Break the writes down to a sensible size. */
		count = (items > SENSIBLE_SIZE) ? SENSIBLE_SIZE : (ssize_t) items ;

		if (ReadFile ((HANDLE) psf->filedes, ((char*) ptr) + total, count, &dwNumberOfBytesRead, 0) == 0)
		{	psf_log_syserr (psf, GetLastError ()) ;
			break ;
			}
		else
			count = dwNumberOfBytesRead ;

		if (count == 0)
			break ;

		total += count ;
		items -= count ;
		} ;

	if (psf->is_pipe)
		psf->pipeoffset += total ;

	return total / bytes ;
} /* psf_fread */

/* Win32 */ sf_count_t
psf_fwrite (const void *ptr, sf_count_t bytes, sf_count_t items, SF_PRIVATE *psf)
{	sf_count_t total = 0 ;
	ssize_t	 count ;
	DWORD dwNumberOfBytesWritten ;

	items *= bytes ;

	/* Do this check after the multiplication above. */
	if (items <= 0)
		return 0 ;

	while (items > 0)
	{	/* Break the writes down to a sensible size. */
		count = (items > SENSIBLE_SIZE) ? SENSIBLE_SIZE : (ssize_t) items ;

		if (WriteFile ((HANDLE) psf->filedes, ((const char*) ptr) + total, count, &dwNumberOfBytesWritten, 0) == 0)
		{	psf_log_syserr (psf, GetLastError ()) ;
			break ;
			}
		else
			count = dwNumberOfBytesWritten ;

		if (count == 0)
			break ;

		total += count ;
		items -= count ;
		} ;

	if (psf->is_pipe)
		psf->pipeoffset += total ;

	return total / bytes ;
} /* psf_fwrite */

/* Win32 */ sf_count_t
psf_ftell (SF_PRIVATE *psf)
{	sf_count_t pos ;
	LONG lDistanceToMoveLow, lDistanceToMoveHigh ;
	DWORD dwResult, dwError ;

	if (psf->is_pipe)
		return psf->pipeoffset ;

	lDistanceToMoveLow = 0 ;
	lDistanceToMoveHigh = 0 ;

	dwResult = SetFilePointer ((HANDLE) psf->filedes, lDistanceToMoveLow, &lDistanceToMoveHigh, FILE_CURRENT) ;

	if (dwResult == 0xFFFFFFFF)
		dwError = GetLastError () ;
	else
		dwError = NO_ERROR ;

	if (dwError != NO_ERROR)
	{	psf_log_syserr (psf, dwError) ;
		return -1 ;
		} ;

	pos = (dwResult + ((__int64) lDistanceToMoveHigh << 32)) ;

	return pos - psf->fileoffset ;
} /* psf_ftell */

/* Win32 */ int
psf_close_fd (int fd)
{	if (CloseHandle ((HANDLE) fd) == 0)
		return -1 ;

	return 0 ;
} /* psf_close_fd */

/* Win32 */ sf_count_t
psf_fgets (char *buffer, sf_count_t bufsize, SF_PRIVATE *psf)
{	sf_count_t k = 0 ;
	sf_count_t count ;
	DWORD dwNumberOfBytesRead ;

	while (k < bufsize - 1)
	{	if (ReadFile ((HANDLE) psf->filedes, &(buffer [k]), 1, &dwNumberOfBytesRead, 0) == 0)
		{	psf_log_syserr (psf, GetLastError ()) ;
			break ;
			}
		else
		{	count = dwNumberOfBytesRead ;
			/* note that we only check for '\n' not other line endings such as CRLF */
			if (count == 0 || buffer [k++] == '\n')
				break ;
			} ;
		} ;

	buffer [k] = 0 ;

	return k ;
} /* psf_fgets */

/* Win32 */ int
psf_is_pipe (SF_PRIVATE *psf)
{	if (GetFileType ((HANDLE) psf->filedes) == FILE_TYPE_DISK)
		return SF_FALSE ;

	/* Default to maximum safety. */
	return SF_TRUE ;
} /* psf_is_pipe */

/* Win32 */ sf_count_t
psf_get_filelen_fd (int fd)
{	sf_count_t filelen ;
	DWORD dwFileSizeLow, dwFileSizeHigh, dwError = NO_ERROR ;

	dwFileSizeLow = GetFileSize ((HANDLE) fd, &dwFileSizeHigh) ;

	if (dwFileSizeLow == 0xFFFFFFFF)
		dwError = GetLastError () ;

	if (dwError != NO_ERROR)
		return (sf_count_t) -1 ;

	filelen = dwFileSizeLow + ((__int64) dwFileSizeHigh << 32) ;

	return filelen ;
} /* psf_get_filelen_fd */

/* Win32 */ int
psf_ftruncate (SF_PRIVATE *psf, sf_count_t len)
{	int retval = 0 ;
	LONG lDistanceToMoveLow, lDistanceToMoveHigh ;
	DWORD dwResult, dwError = NO_ERROR ;

	/* This implementation trashes the current file position.
	** should it save and restore it? what if the current position is past
	** the new end of file?
	*/

	/* Returns 0 on success, non-zero on failure. */
	if (len < 0)
		return 1 ;

	lDistanceToMoveLow = (DWORD) (len & 0xFFFFFFFF) ;
	lDistanceToMoveHigh = (DWORD) ((len >> 32) & 0xFFFFFFFF) ;

	dwResult = SetFilePointer ((HANDLE) psf->filedes, lDistanceToMoveLow, &lDistanceToMoveHigh, FILE_BEGIN) ;

	if (dwResult == 0xFFFFFFFF)
		dwError = GetLastError () ;

	if (dwError != NO_ERROR)
	{	retval = -1 ;
		psf_log_syserr (psf, dwError) ;
		}
	else
	{	/* Note: when SetEndOfFile is used to extend a file, the contents of the
		** new portion of the file is undefined. This is unlike chsize(),
		** which guarantees that the new portion of the file will be zeroed.
		** Not sure if this is important or not.
		*/
		if (SetEndOfFile ((HANDLE) psf->filedes) == 0)
		{	retval = -1 ;
			psf_log_syserr (psf, GetLastError ()) ;
			} ;
		} ;

	return retval ;
} /* psf_ftruncate */


#else
/* Win32 file i/o functions implemented using Unix-style file i/o API */

/* Win32 has a 64 file offset seek function:
**
**		__int64 _lseeki64 (int handle, __int64 offset, int origin) ;
**
** It also has a 64 bit fstat function:
**
**		int fstati64 (int, struct _stati64) ;
**
** but the fscking thing doesn't work!!!!! The file size parameter returned
** by this function is only valid up until more data is written at the end of
** the file. That makes this function completely 100% useless.
*/

#include <io.h>
#include <direct.h>

#ifndef HAVE_SSIZE_T
typedef long ssize_t ;
#endif

/* Win32 */ int
psf_fopen (SF_PRIVATE *psf, const char *pathname, int open_mode)
{	int oflag, mode ;

	switch (open_mode)
	{	case SFM_READ :
				oflag = O_RDONLY | O_BINARY ;
				mode = 0 ;
				break ;

		case SFM_WRITE :
				oflag = O_WRONLY | O_CREAT | O_TRUNC | O_BINARY ;
				mode = S_IRUSR | S_IWUSR | S_IRGRP ;
				break ;

		case SFM_RDWR :
				oflag = O_RDWR | O_CREAT | O_BINARY ;
				mode = S_IRUSR | S_IWUSR | S_IRGRP ;
				break ;

		default :
				psf->error = SFE_BAD_OPEN_MODE ;
				return -1 ;
				break ;
		} ;

	if (mode == 0)
		psf->filedes = open (pathname, oflag) ;
	else
		psf->filedes = open (pathname, oflag, mode) ;

	if (psf->filedes == -1)
		psf_log_syserr (psf, errno) ;

	return psf->filedes ;
} /* psf_fopen */

/* Win32 */ sf_count_t
psf_fseek (SF_PRIVATE *psf, sf_count_t offset, int whence)
{	sf_count_t	new_position ;

	switch (whence)
	{	case SEEK_SET :
				offset += psf->fileoffset ;
				break ;

		case SEEK_END :
				if (psf->mode == SFM_WRITE)
				{	new_position = _lseeki64 (psf->filedes, offset, whence) ;

					if (new_position < 0)
						psf_log_syserr (psf, errno) ;

					return new_position - psf->fileoffset ;
					} ;

				/* Transform SEEK_END into a SEEK_SET, ie find the file
				** length add the requested offset (should be <= 0) to
				** get the offset wrt the start of file.
				*/
				whence = SEEK_SET ;
				offset = _lseeki64 (psf->filedes, 0, SEEK_END) + offset ;
				break ;

		default :
				/* No need to do anything about SEEK_CUR. */
				break ;
		} ;

	/*
	** Bypass weird Win32-ism if necessary.
	** _lseeki64() returns an "invalid parameter" error if called with the
	** offset == 0 and whence == SEEK_CUR.
	*** Use the _telli64() function instead.
	*/
	if (offset == 0 && whence == SEEK_CUR)
		new_position = _telli64 (psf->filedes) ;
	else
		new_position = _lseeki64 (psf->filedes, offset, whence) ;

	if (new_position < 0)
		psf_log_syserr (psf, errno) ;

	new_position -= psf->fileoffset ;

	return new_position ;
} /* psf_fseek */

/* Win32 */ sf_count_t
psf_fread (void *ptr, sf_count_t bytes, sf_count_t items, SF_PRIVATE *psf)
{	sf_count_t total = 0 ;
	ssize_t	 count ;

	items *= bytes ;

	/* Do this check after the multiplication above. */
	if (items <= 0)
		return 0 ;

	while (items > 0)
	{	/* Break the writes down to a sensible size. */
		count = (items > SENSIBLE_SIZE) ? SENSIBLE_SIZE : (ssize_t) items ;

		count = read (psf->filedes, ((char*) ptr) + total, (size_t) count) ;

		if (count == -1)
		{	if (errno == EINTR)
				continue ;

			psf_log_syserr (psf, errno) ;
			break ;
			} ;

		if (count == 0)
			break ;

		total += count ;
		items -= count ;
		} ;

	return total / bytes ;
} /* psf_fread */

/* Win32 */ sf_count_t
psf_fwrite (const void *ptr, sf_count_t bytes, sf_count_t items, SF_PRIVATE *psf)
{	sf_count_t total = 0 ;
	ssize_t	 count ;

	items *= bytes ;

	/* Do this check after the multiplication above. */
	if (items <= 0)
		return 0 ;

	while (items > 0)
	{	/* Break the writes down to a sensible size. */
		count = (items > SENSIBLE_SIZE) ? SENSIBLE_SIZE : items ;

		count = write (psf->filedes, ((const char*) ptr) + total, count) ;

		if (count == -1)
		{	if (errno == EINTR)
				continue ;

			psf_log_syserr (psf, errno) ;
			break ;
			} ;

		if (count == 0)
			break ;

		total += count ;
		items -= count ;
		} ;

	return total / bytes ;
} /* psf_fwrite */

/* Win32 */ sf_count_t
psf_ftell (SF_PRIVATE *psf)
{	sf_count_t pos ;

	pos = _telli64 (psf->filedes) ;

	if (pos == ((sf_count_t) -1))
	{	psf_log_syserr (psf, errno) ;
		return -1 ;
		} ;

	return pos - psf->fileoffset ;
} /* psf_ftell */

/* Win32 */ int
psf_fclose (SF_PRIVATE *psf)
{	int retval ;

	while ((retval = close (psf->filedes)) == -1 && errno == EINTR)
		/* Do nothing. */ ;

	if (retval == -1)
		psf_log_syserr (psf, errno) ;

	psf->filedes = -1 ;

	return retval ;
} /* psf_fclose */

/* Win32 */ sf_count_t
psf_fgets (char *buffer, sf_count_t bufsize, SF_PRIVATE *psf)
{	sf_count_t	k = 0 ;
	sf_count_t	count ;

	while (k < bufsize - 1)
	{	count = read (psf->filedes, &(buffer [k]), 1) ;

		if (count == -1)
		{	if (errno == EINTR)
				continue ;

			psf_log_syserr (psf, errno) ;
			break ;
			} ;

		if (count == 0 || buffer [k++] == '\n')
			break ;
		} ;

	buffer [k] = 0 ;

	return k ;
} /* psf_fgets */

/* Win32 */ int
psf_is_pipe (SF_PRIVATE *psf)
{	struct stat statbuf ;

	/* Not sure if this works. */

	if (fstat (psf->filedes, &statbuf) == -1)
	{	psf_log_syserr (psf, errno) ;
		/* Default to maximum safety. */
		return SF_TRUE ;
		} ;

	/* These macros are defined in Win32/unistd.h. */
	if (S_ISFIFO (statbuf.st_mode) || S_ISSOCK (statbuf.st_mode))
		return SF_TRUE ;

	return SF_FALSE ;
} /* psf_checkpipe */

/* Win32 */ sf_count_t
psf_get_filelen (SF_PRIVATE *psf)
{
#if 0
	/*
	** Windoze is SOOOOO FUCKED!!!!!!!
	** This code should work but doesn't. Why?
	** Code below does work.
	*/
	struct _stati64 statbuf ;

	if (_fstati64 (psf->filedes, &statbuf))
	{	psf_log_syserr (psf, errno) ;
		return (sf_count_t) -1 ;
		} ;

	return statbuf.st_size ;
#else
	sf_count_t current, filelen ;

	if ((current = _telli64 (psf->filedes)) < 0)
	{	psf_log_syserr (psf, errno) ;
		return (sf_count_t) -1 ;
		} ;

	/*
	** Lets face it, windoze if FUBAR!!!
	**
	** For some reason, I have to call _lseeki64() TWICE to get to the
	** end of the file.
	**
	** This might have been avoided if windows had implemented the POSIX
	** standard function fsync() but NO, that would have been too easy.
	**
	** I am VERY close to saying that windoze will no longer be supported
	** by libsndfile and changing the license to GPL at the same time.
	*/

	_lseeki64 (psf->filedes, 0, SEEK_END) ;

	if ((filelen = _lseeki64 (psf->filedes, 0, SEEK_END)) < 0)
	{	psf_log_syserr (psf, errno) ;
		return (sf_count_t) -1 ;
		} ;

	if (filelen > current)
		_lseeki64 (psf->filedes, current, SEEK_SET) ;

	switch (psf->mode)
	{	case SFM_WRITE :
			filelen = filelen - psf->fileoffset ;
			break ;

		case SFM_READ :
			if (psf->fileoffset > 0 && psf->filelength > 0)
				filelen = psf->filelength ;
			break ;

		case SFM_RDWR :
			/*
			** Cannot open embedded files SFM_RDWR so we don't need to
			** subtract psf->fileoffset. We already have the answer we
			** need.
			*/
			break ;

		default :
			filelen = 0 ;
		} ;

	return filelen ;
#endif
} /* psf_get_filelen */

/* Win32 */ int
psf_ftruncate (SF_PRIVATE *psf, sf_count_t len)
{	int retval ;

	/* Returns 0 on success, non-zero on failure. */
	if (len < 0)
		return 1 ;

	/* The global village idiots at micorsoft decided to implement
	** nearly all the required 64 bit file offset functions except
	** for one, truncate. The fscking morons!
	**
	** This is not 64 bit file offset clean. Somone needs to clean
	** this up.
	*/
	if (len > 0x7FFFFFFF)
		return -1 ;

	retval = chsize (psf->filedes, len) ;

	if (retval == -1)
		psf_log_syserr (psf, errno) ;

	return retval ;
} /* psf_ftruncate */


static void
psf_log_syserr (SF_PRIVATE *psf, int error)
{
	/* Only log an error if no error has been set yet. */
	if (psf->error == 0)
	{	psf->error = SFE_SYSTEM ;
		LSF_SNPRINTF (psf->syserr, sizeof (psf->syserr), "System error : %s", strerror (error)) ;
		} ;

	return ;
} /* psf_log_syserr */

#endif

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: 749740d7-ecc7-47bd-8cf7-600f31d32e6d
*/
