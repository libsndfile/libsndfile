/*
** Copyright (C) 2005,2006 Erik de Castro Lopo <erikd@mega-nerd.com>
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

/*
** The above modified BSD style license (GPL and LGPL compatible) applies to
** this file. It does not apply to libsndfile itself which is released under
** the GNU LGPL or the libsndfile test suite which is released under the GNU
** GPL.
** This means that this header file can be used under this modified BSD style
** license, but the LGPL still holds for the libsndfile library itself.
*/

/*
** sndfile.hh -- A lightweight C++ wrapper for the libsndfile API.
**
** All the methods are inlines and all functionality is contained in this
** file. There is no separate implementation file.
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
		static const SF_INFO zero_sfinfo ;

			/* Default constructor */
			Sndfile (void) ;
			Sndfile (const char *path, int mode, const SF_INFO *sfinfo_in) ;
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

		template <typename T> sf_count_t read (T *ptr, sf_count_t items) ;

		template <typename T> sf_count_t write (const T *ptr, sf_count_t items) ;

		template <typename T> sf_count_t readf (T *ptr, sf_count_t frames) ;

		template <typename T> sf_count_t writef (const T *ptr, sf_count_t frames) ;

		void	close		(void) ;
} ;

/*==============================================================================
**	Nothing but implementation below.
*/

const SF_INFO Sndfile::zero_sfinfo = { 0, 0, 0, 0, 0, 0 } ;

inline
Sndfile::Sndfile (void)
: psf (NULL), sfinfo (zero_sfinfo)
{
} /* Sndfile constructor */

inline
Sndfile::Sndfile (const char *path, int mode, const SF_INFO *sfinfo_in)
: psf (NULL), sfinfo (sfinfo_in ? *sfinfo_in : zero_sfinfo)
{	psf = sf_open (path, mode, &sfinfo) ;
} /* Sndfile constructor */

inline
Sndfile::~Sndfile (void)
{	if (psf != NULL)
		sf_close (psf) ;
	psf = NULL ;
} /* Sndfile destructor */

inline bool
Sndfile::openRead (const char *path)
{	close () ;
	psf = sf_open (path, SFM_READ, &sfinfo) ;
	if (psf == NULL)
		return false ;
	return true ;
} /* Sndfile::openRead */

inline bool
Sndfile::openWrite (const char *path)
{	close () ;
	psf = sf_open (path, SFM_WRITE, &sfinfo) ;
	if (psf == NULL)
		return false ;
	return true ;
} /* Sndfile::openWrite */

inline bool
Sndfile::openReadWrite (const char *path)
{	close () ;
	psf = sf_open (path, SFM_RDWR, &sfinfo) ;
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

template <>
inline sf_count_t
Sndfile::read <short> (short *ptr, sf_count_t items)
{	return sf_read_short (psf, ptr, items) ; }

template <>
inline sf_count_t
Sndfile::read <int> (int *ptr, sf_count_t items)
{	return sf_read_int (psf, ptr, items) ; }

template <>
inline sf_count_t
Sndfile::read <float> (float *ptr, sf_count_t items)
{	return sf_read_float (psf, ptr, items) ; }

template <>
inline sf_count_t
Sndfile::read <double> (double *ptr, sf_count_t items)
{	return sf_read_double (psf, ptr, items) ; }

template <>
inline sf_count_t
Sndfile::write <short> (const short *ptr, sf_count_t items)
{	return sf_write_short (psf, ptr, items) ; }

template <>
inline sf_count_t
Sndfile::write <int> (const int *ptr, sf_count_t items)
{	return sf_write_int (psf, ptr, items) ; }

template <>
inline sf_count_t
Sndfile::write <float> (const float *ptr, sf_count_t items)
{	return sf_write_float (psf, ptr, items) ; }

template <>
inline sf_count_t
Sndfile::write <double> (const double *ptr, sf_count_t items)
{	return sf_write_double (psf, ptr, items) ; }

template <>
inline sf_count_t
Sndfile::readf <short> (short *ptr, sf_count_t frames)
{	return sf_readf_short (psf, ptr, frames) ; }

template <>
inline sf_count_t
Sndfile::readf <int> (int *ptr, sf_count_t frames)
{	return sf_readf_int (psf, ptr, frames) ; }

template <>
inline sf_count_t
Sndfile::readf <float> (float *ptr, sf_count_t frames)
{	return sf_readf_float (psf, ptr, frames) ; }

template <>
inline sf_count_t
Sndfile::readf <double> (double *ptr, sf_count_t frames)
{	return sf_readf_double (psf, ptr, frames) ; }

template <>
inline sf_count_t
Sndfile::writef <short> (const short *ptr, sf_count_t frames)
{	return sf_writef_short (psf, ptr, frames) ; }

template <>
inline sf_count_t
Sndfile::writef <int> (const int *ptr, sf_count_t frames)
{	return sf_writef_int (psf, ptr, frames) ; }

template <>
inline sf_count_t
Sndfile::writef <float> (const float *ptr, sf_count_t frames)
{	return sf_writef_float (psf, ptr, frames) ; }

template <>
inline sf_count_t
Sndfile::writef <double> (const double *ptr, sf_count_t frames)
{	return sf_writef_double (psf, ptr, frames) ; }

#endif	/* SNDFILE_HH */

/*
** Do not edit or modify anything in this comment block.
** The following line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: a0e9d996-73d7-47c4-a78d-79a3232a9eef
*/
