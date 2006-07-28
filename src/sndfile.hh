/*
** Copyright (C) 2005,2006 Erik de Castro Lopo <erikd@mega-nerd.com>
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

class Sndfile
{	private :
		SNDFILE *psf ;

		/* Copy constructor and assignment operators are not supported. */
		Sndfile (const Sndfile&) ;
		Sndfile & operator = (const Sndfile&) ;

	public :
		SF_INFO sfinfo ;

			/* Default constructor */
			Sndfile (void) ;
			Sndfile (const char *path, int mode, SF_INFO *sfinfo_in) ;
			~Sndfile (void) ;

		bool openRead (const char *path) ;
		bool openWrite (const char *path) ;
		bool openReadWrite (const char *path) ;

		int error (void) ;
		const char * strError (void) ;

		int command (int cmd, void *data, int datasize) ;

		sf_count_t	seek (sf_count_t frames, int whence) ;

		int setString (int str_type, const char* str) ;

		const char* getString (int str_type) ;

		sf_count_t	read	(short *ptr, sf_count_t items) ;
		sf_count_t	read	(int *ptr, sf_count_t items) ;
		sf_count_t	read	(float *ptr, sf_count_t items) ;
		sf_count_t	read	(double *ptr, sf_count_t items) ;

		sf_count_t	write	(const short *ptr, sf_count_t items) ;
		sf_count_t	write	(const int *ptr, sf_count_t items) ;
		sf_count_t	write	(const float *ptr, sf_count_t items) ;
		sf_count_t	write	(const double *ptr, sf_count_t items) ;

		sf_count_t	readf	(short *ptr, sf_count_t frames) ;
		sf_count_t	readf	(int *ptr, sf_count_t frames) ;
		sf_count_t	readf	(float *ptr, sf_count_t frames) ;
		sf_count_t	readf	(double *ptr, sf_count_t frames) ;

		sf_count_t	writef	(const short *ptr, sf_count_t frames) ;
		sf_count_t	writef	(const int *ptr, sf_count_t frames) ;
		sf_count_t	writef	(const float *ptr, sf_count_t frames) ;
		sf_count_t	writef	(const double *ptr, sf_count_t frames) ;

		void	close		(void) ;
} ;

/*==============================================================================
**	Nothing but implementation below.
*/

inline
Sndfile::Sndfile (void) : psf (NULL)
{	static SF_INFO zero_sfinfo = { 0, 0, 0, 0, 0, 0 } ;
	sfinfo = zero_sfinfo ;
} /* Sndfile constructor */
	

inline
Sndfile::Sndfile (const char *path, int mode, SF_INFO *sfinfo_in) : psf (NULL)
{	sfinfo = *sfinfo_in ;
	psf = sf_open (path, mode, &sfinfo) ;
} /* Sndfile constructor */

inline
Sndfile::~Sndfile (void)
{	if (psf != NULL)
		sf_close (psf) ;
	psf = NULL ;
} /* Sndfile destructor */

inline bool
Sndfile::openRead (const char *path)
{	psf = sf_open (path, SFM_READ, &sfinfo) ;
	if (psf == NULL)
		return false ;
	return true ;
} /* Sndfile::openRead */

inline bool
Sndfile::openWrite (const char *path)
{	psf = sf_open (path, SFM_WRITE, &sfinfo) ;
	if (psf == NULL)
		return false ;
	return true ;
} /* Sndfile::openWrite */

inline bool
Sndfile::openReadWrite (const char *path)
{	psf = sf_open (path, SFM_RDWR, &sfinfo) ;
	if (psf == NULL)
		return false ;
	return true ;
} /* Sndfile::openReadWrite */

inline void
Sndfile::close (void)
{	if (psf != NULL)
		sf_close (psf) ;
	psf = NULL ;
} /* Sndfile::close */

inline int
Sndfile::error (void)
{	return sf_error (psf) ; }

inline const char *
Sndfile::strError (void)
{	return sf_strerror (psf) ; }

inline int
Sndfile::command (int cmd, void *data, int datasize)
{	return sf_command (psf, cmd, data, datasize) ; }

inline sf_count_t
Sndfile::seek (sf_count_t frames, int whence)
{	return sf_seek (psf, frames, whence) ; }

inline int
Sndfile::setString (int str_type, const char* str)
{	return sf_set_string (psf, str_type, str) ; }

inline const char*
Sndfile::getString (int str_type)
{	return sf_get_string (psf, str_type) ; }


/*---------------------------------------------------------------------*/

inline sf_count_t
Sndfile::read	(short *ptr, sf_count_t items)
{	return sf_read_short (psf, ptr, items) ; }

inline sf_count_t
Sndfile::read	(int *ptr, sf_count_t items)
{	return sf_read_int (psf, ptr, items) ; }

inline sf_count_t
Sndfile::read	(float *ptr, sf_count_t items)
{	return sf_read_float (psf, ptr, items) ; }

inline sf_count_t
Sndfile::read	(double *ptr, sf_count_t items)
{	return sf_read_double (psf, ptr, items) ; }

inline sf_count_t
Sndfile::write	(const short *ptr, sf_count_t items)
{	return sf_write_short (psf, ptr, items) ; }

inline sf_count_t
Sndfile::write	(const int *ptr, sf_count_t items)
{	return sf_write_int (psf, ptr, items) ; }

inline sf_count_t
Sndfile::write	(const float *ptr, sf_count_t items)
{	return sf_write_float (psf, ptr, items) ; }

inline sf_count_t
Sndfile::write	(const double *ptr, sf_count_t items)
{	return sf_write_double (psf, ptr, items) ; }

inline sf_count_t
Sndfile::readf	(short *ptr, sf_count_t frames)
{	return sf_readf_short (psf, ptr, frames) ; }

inline sf_count_t
Sndfile::readf	(int *ptr, sf_count_t frames)
{	return sf_readf_int (psf, ptr, frames) ; }

inline sf_count_t
Sndfile::readf	(float *ptr, sf_count_t frames)
{	return sf_readf_float (psf, ptr, frames) ; }

inline sf_count_t
Sndfile::readf	(double *ptr, sf_count_t frames)
{	return sf_readf_double (psf, ptr, frames) ; }

inline sf_count_t
Sndfile::writef	(const short *ptr, sf_count_t frames)
{	return sf_writef_short (psf, ptr, frames) ; }

inline sf_count_t
Sndfile::writef	(const int *ptr, sf_count_t frames)
{	return sf_writef_int (psf, ptr, frames) ; }

inline sf_count_t
Sndfile::writef	(const float *ptr, sf_count_t frames)
{	return sf_writef_float (psf, ptr, frames) ; }

inline sf_count_t
Sndfile::writef	(const double *ptr, sf_count_t frames)
{	return sf_writef_double (psf, ptr, frames) ; }

#endif	/* SNDFILE_HH */

/*
** Do not edit or modify anything in this comment block.
** The following line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: a0e9d996-73d7-47c4-a78d-79a3232a9eef
*/
