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
static	int		mp3_close		(SF_PRIVATE *psf) ;
static  int     mp3_read_header (SF_PRIVATE * psf, mpg123_handle * decoder) ;

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

    psf->codec_data = NULL;

    // TODO: writing!
	if (psf->file.mode == SFM_WRITE || psf->file.mode == SFM_RDWR)
		return SFE_UNIMPLEMENTED ;

    if (mpg123_initialised == 0) {
        int decoder_init_err = mpg123_init();
        // FIXME: find somewhere to put mpg123_exit() call
        if (decoder_init_err != MPG123_OK) {
            psf_log_printf(psf, "Failed to init mpg123.\n");
            return SFE_UNIMPLEMENTED ; // FIXME: semantically wrong return code
        }
    }
    mpg123_initialised++;

    decoder = mpg123_new(NULL, &decoder_err);

    if (decoder_err != MPG123_OK) {
        decoder_err = mpg123_open_feed(decoder);
    }

    if (decoder_err != MPG123_OK) {
        decoder_err = mp3_read_header(psf, decoder);
    }

    if (decoder_err != MPG123_OK) {
        mp3_close(NULL);
        psf_log_printf(psf, "Failed to initialise mp3 decoder.\n");
        return SFE_UNIMPLEMENTED ; // FIXME: semantically wrong return code
    }
    psf->codec_data = decoder;
	psf->container_close = mp3_close ;
    // TODO: seeking
    psf->sf.seekable = 0;


    // FIXME: This block:
    return SFE_UNIMPLEMENTED;

    return error;
}

/*------------------------------------------------------------------------------
*/

static int
mp3_close (SF_PRIVATE * psf)
{
    mpg123_handle * decoder = psf->codec_data;
    if (decoder != NULL) {
        // mpg123_close(decoder); <- Not sure if we need this?
        mpg123_delete(decoder);
    }
    if (!--mpg123_initialised) {
        mpg123_exit();
    }
    return 0;
}

static int
mp3_read_header (SF_PRIVATE * psf, mpg123_handle * decoder) {
    int decoder_err;
    size_t n_bytes_read;
    char buffer;
    do {
        // TODO: is reading a byte at a time required?
        n_bytes_read = psf_fread(&buffer, 1, 1, psf);
        if (n_bytes_read == 0) {
            decoder_err = MPG123_DONE; // FIXME: Backchannel error
        }
        decoder_err = mpg123_decode(
            decoder, (unsigned char *) &buffer, 1, NULL, 0, &n_bytes_read);
    } while (decoder_err == MPG123_NEED_MORE);
    return decoder_err;
}
