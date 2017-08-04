/*
** Copyright (C) 2002-2011 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include	"sfconfig.h"

#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<ctype.h>
#include    <mpg123.h>

#include	"sndfile.h"
#include	"sfendian.h"
#include	"common.h"

/*------------------------------------------------------------------------------
** Macros to handle big/little endian issues.
*/

/*------------------------------------------------------------------------------
** Typedefs.
*/

/*------------------------------------------------------------------------------
** Private static functions.
*/

// FIXME: This initialisation should have a better hook
static int mpg123_initialised = 0 ;

/*------------------------------------------------------------------------------
** Public function.
*/

// FIXME: use mpg123 error string reporting
int
mp3_open (SF_PRIVATE * psf)
{
    int error = 0;
    int decoder_err;
    mpg123_handle * decoder;

	if (psf->file.mode == SFM_WRITE || psf->file.mode == SFM_RDWR)
		return SFE_UNIMPLEMENTED ;

    if (mpg123_initialised == 0) {
        int decoder_init_err = mpg123_init();
        // FIXME: find somewhere to put mpg123_exit() call
        if (decoder_init_err == MPG123_OK) {
            mpg123_initialised = 1;
        } else {
            psf_log_printf(psf, "Failed to init mpg123.\n");
            return SFE_UNIMPLEMENTED ; // FIXME: semantically wrong return code
        }
    }

    decoder = mpg123_new(NULL, &decoder_err);
    
    if (decoder == NULL) {
        psf_log_printf(psf, "Failed to initialise mp3 decoder: %s");
        return SFE_UNIMPLEMENTED ; // FIXME: semantically wrong return code
    }

    // FIXME: This block:
    mpg123_close(decoder);
    mpg123_delete(decoder);
    mpg123_exit();
    return SFE_UNIMPLEMENTED;

    return error;
}

/*------------------------------------------------------------------------------
*/
