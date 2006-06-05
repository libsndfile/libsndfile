/*
** Copyright (C) 2005 Erik de Castro Lopo <erikd@mega-nerd.com>
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
** sndfile.hh -- A C++ wrapper for the libsndfile API.
**
** API documentation is in the doc/ directory of the source code tarball
** and at http://www.mega-nerd.com/libsndfile/api.html.
*/

#ifndef SNDFILE_HH
#define SNDFILE_HH

#include <sndfile.h>

class SndFile
{	private :
		SNDFILE *psf ;

	public :
		SF_INFO sfinfo ;

			/* Default constructor */
			SndFile (void) : psf (NULL) {}
			~SndFile (void) ;

		bool OpenRead (const char *path) ;
		bool OpenWrite (const char *path) ;
		bool OpenReadWrite (const char *path) ;

		int Error (void) ;
		const char * StrError (void) ;
		
		int Command (int command, void *data, int datasize) ;

		sf_count_t	Seek (sf_count_t frames, int whence) ;

		int SetString (int str_type, const char* str) ;

		const char* GetString (int str_type) ;

		sf_count_t	Read	(short *ptr, sf_count_t items) ;
		sf_count_t	Read	(int *ptr, sf_count_t items) ;
		sf_count_t	Read	(float *ptr, sf_count_t items) ;
		sf_count_t	Read	(double *ptr, sf_count_t items) ;

		sf_count_t	Write	(const short *ptr, sf_count_t items) ;
		sf_count_t	Write	(const int *ptr, sf_count_t items) ;
		sf_count_t	Write	(const float *ptr, sf_count_t items) ;
		sf_count_t	Write	(const double *ptr, sf_count_t items) ;

		sf_count_t	ReadF	(short *ptr, sf_count_t frames) ;
		sf_count_t	ReadF	(int *ptr, sf_count_t frames) ;
		sf_count_t	ReadF	(float *ptr, sf_count_t frames) ;
		sf_count_t	ReadF	(double *ptr, sf_count_t frames) ;

		sf_count_t	WriteF	(const short *ptr, sf_count_t frames) ;
		sf_count_t	WriteF	(const int *ptr, sf_count_t frames) ;
		sf_count_t	WriteF	(const float *ptr, sf_count_t frames) ;
		sf_count_t	WriteF	(const double *ptr, sf_count_t frames) ;

		int		Close		(void) ;
} ;

SndFile::SndFile (const char *path, int mode, SF_INFO *sfinfo)
{	psf = sf_open (path, mode, sfinfo) ;
} /* SndFile constructor */

SndFile::~SndFile (void)
{	if (psf != NULL)
		sf_close (psf) ;
	psf = NULL ;
} /* SndFile destructor */

bool
SndFile::OpenRead (const char *path)
{	psf = sf_open (path, SFM_READ, &sfinfo) ;
	if (psf == NULL)
		return false ;
	return true ;
} /* SndFile::OpenRead */

bool
SndFile::OpenWrite (const char *path)
{	psf = sf_open (path, SFM_WRITE, &sfinfo) ;
	if (psf == NULL)
		return false ;
	return true ;
} /* SndFile::OpenWrite */

bool
SndFile::OpenReadWrite (const char *path)
{	psf = sf_open (path, SFM_RDWR, &sfinfo) ;
	if (psf == NULL)
		return false ;
	return true ;
} /* SndFile::OpenReadWrite */

#endif	/* SNDFILE_HH */

/*
** Do not edit or modify anything in this comment block.
** The following line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: a0e9d996-73d7-47c4-a78d-79a3232a9eef
*/
