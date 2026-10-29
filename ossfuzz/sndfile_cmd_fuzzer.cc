/*
** sndfile_cmd_fuzzer.cc -- sf_command surface + SFM_RDWR header-update harness.
**
** Threat model
** ------------
** Untrusted input: the byte stream; the leading part stands in for an
** attacker-supplied audio file, the trailing 4 bytes stand in for the
** application-controlled command parameters (dither config, metadata
** strings, update modes).  Converter/editor tools feed exactly such
** untrusted files while their users (or downloaded metadata) drive
** sf_command() calls, so both halves are attacker-influenced in practice.
** Trust boundary: sf_open_virtual (SFM_RDWR, falling back to SFM_READ) on
** an in-memory file plus the documented sf_command()/sf_seek()/
** sf_writef_short() API.  All struct contents are bounded and NUL-kept so
** the API contract is honoured.
**
** Rationale
** ---------
** No harness drives the sf_command() control surface: src/command.c (125
** lines) is 0 % covered, as are broadcast.c (bext chunk writer, 94 dark),
** the cart chunk writer, dither.c (234, read+write config), the
** instrument/cue/loop metadata paths, and the SFM_RDWR in-place header
** update/rewrite paths (part of sndfile.c's 1700+ dark lines).  This
** harness opens a file for RDWR (real in-memory write vio), drives a
** bounded subset of SFC_* commands derived from the control bytes, seeks,
** writes a few frames, forces header updates and closes -- exercising the
** full metadata + header-rewrite machinery against fuzzed files.
**
** Seeds: a handful of minimal valid containers (WAV/AIFF/AU/CAF/NIST) so
** the harness gets past header parsing; dictionary: container and chunk
** FourCCs so mutation can reach the chunk walkers in the file half.
**
** Expected bug classes: fixed-field string handling (bext/cart), header
** size arithmetic on rewrite (integer overflow), instrument/cue metadata
** parsing, dither state misuse, RDWR seek/truncate errors, leaks on error
** paths.
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>
#include <inttypes.h>

/* In-memory virtual file with a real write direction. */
typedef struct
{	unsigned char *data ;
	sf_count_t length ;
	sf_count_t offset ;
} CMD_VIO ;

static sf_count_t cmd_get_filelen (void *user_data)
{	CMD_VIO *vf = (CMD_VIO *) user_data ;
	return vf->length ;
} /* cmd_get_filelen */

static sf_count_t cmd_seek (sf_count_t offset, int whence, void *user_data)
{	CMD_VIO *vf = (CMD_VIO *) user_data ;

	switch (whence)
	{	case SEEK_SET : vf->offset = offset ; break ;
		case SEEK_CUR : vf->offset += offset ; break ;
		case SEEK_END : vf->offset = vf->length + offset ; break ;
		default : return -1 ;
		} ;

	if (vf->offset > vf->length)
		vf->offset = vf->length ;
	if (vf->offset < 0)
		vf->offset = 0 ;

	return vf->offset ;
} /* cmd_seek */

static sf_count_t cmd_read (void *ptr, sf_count_t count, void *user_data)
{	CMD_VIO *vf = (CMD_VIO *) user_data ;

	if (vf->offset + count > vf->length)
		count = vf->length - vf->offset ;
	if (count < 0)
		return 0 ;

	memcpy (ptr, vf->data + vf->offset, count) ;
	vf->offset += count ;

	return count ;
} /* cmd_read */

static sf_count_t cmd_write (const void *ptr, sf_count_t count, void *user_data)
{	CMD_VIO *vf = (CMD_VIO *) user_data ;

	/* Writes may only overwrite existing bytes (header updates, in-place
	** data edits); libsndfile's RDWR mode never grows the file. */
	if (vf->offset + count > vf->length)
		count = vf->length - vf->offset ;
	if (count < 0)
		return 0 ;

	memcpy (vf->data + vf->offset, ptr, count) ;
	vf->offset += count ;

	return count ;
} /* cmd_write */

static sf_count_t cmd_tell (void *user_data)
{	CMD_VIO *vf = (CMD_VIO *) user_data ;
	return vf->offset ;
} /* cmd_tell */

static volatile double cmd_sink = 0.0 ;

static void cmd_drive_commands (SNDFILE *sndfile, const uint8_t ctl [4], sf_count_t frames, int channels)
{	SF_DITHER_INFO dither ;
	SF_FORMAT_INFO fmt_info ;
	SF_INSTRUMENT inst ;
	SF_LOOP_INFO loop_info ;
	SF_BROADCAST_INFO *bext ;
	SF_CUES cues ;
	SF_CART_INFO *cart ;
	double dval [256] ;/* bounded: larger channel files simply return SF_FALSE */
	int count, idval, simple_count, k ;

	(void) frames ;
	memset (dval, 0, sizeof (dval)) ;

	/* Normalisation / conversion / clipping switches. */
	sf_command (sndfile, SFC_SET_NORM_FLOAT, NULL, (ctl [1] >> 6) & 1) ;
	sf_command (sndfile, SFC_SET_NORM_DOUBLE, NULL, (ctl [1] >> 6) & 1) ;
	sf_command (sndfile, SFC_SET_SCALE_FLOAT_INT_READ, NULL, (ctl [1] >> 7) & 1) ;
	sf_command (sndfile, SFC_SET_SCALE_INT_FLOAT_WRITE, NULL, (ctl [2] >> 1) & 1) ;
	sf_command (sndfile, SFC_SET_CLIPPING, NULL, ctl [2] & 1) ;
	sf_command (sndfile, SFC_GET_CLIPPING, &idval, sizeof (idval)) ;
	cmd_sink += idval ;
	sf_command (sndfile, SFC_RAW_DATA_NEEDS_ENDSWAP, NULL, (ctl [2] >> 2) & 1) ;

	/* Dither on read and write (dither.c is 0 % covered today). */
	memset (&dither, 0, sizeof (dither)) ;
	dither.type = SFD_DEFAULT_LEVEL + ((ctl [1] >> 4) & 3) * 500 ;
	dither.level = 1.0 ;
	sf_command (sndfile, SFC_SET_DITHER_ON_READ, &dither, sizeof (dither)) ;
	sf_command (sndfile, SFC_SET_DITHER_ON_WRITE, &dither, sizeof (dither)) ;

	/* Header update modes. */
	sf_command (sndfile, SFC_SET_ADD_PEAK_CHUNK, NULL, ctl [1] & 1) ;
	sf_command (sndfile, SFC_SET_UPDATE_HEADER_AUTO, NULL, (ctl [1] >> 1) & 1) ;
	sf_command (sndfile, SFC_SET_ADD_HEADER_PAD_CHUNK, NULL, (ctl [1] >> 3) & 1) ;
	if ((ctl [1] >> 2) & 1)
		sf_command (sndfile, SFC_UPDATE_HEADER_NOW, NULL, 0) ;

	if (ctl [0] & 1)
	{	/* Peak calculations walk every frame of every channel. */
		sf_command (sndfile, SFC_CALC_SIGNAL_MAX, &dval [0], sizeof (double)) ;
		sf_command (sndfile, SFC_CALC_NORM_SIGNAL_MAX, &dval [0], sizeof (double)) ;
		/* datalen must be exactly channels * sizeof (double). */
		if (channels >= 1 && channels <= 256)
		{	sf_command (sndfile, SFC_CALC_MAX_ALL_CHANNELS, dval, sizeof (double) * channels) ;
			sf_command (sndfile, SFC_CALC_NORM_MAX_ALL_CHANNELS, dval, sizeof (double) * channels) ;
			} ;
		cmd_sink += dval [0] ;
	} ;

	if (ctl [0] & 2)
	{	/* Instrument metadata: get, then set a bounded modification. */
		memset (&inst, 0, sizeof (inst)) ;
		if (sf_command (sndfile, SFC_GET_INSTRUMENT, &inst, sizeof (inst)))
		{	inst.gain = (int) ctl [2] - 128 ;
			inst.basenote = 60 ;
			inst.loop_count = 0 ;
			sf_command (sndfile, SFC_SET_INSTRUMENT, &inst, sizeof (inst)) ;
		} ;

		memset (&loop_info, 0, sizeof (loop_info)) ;
		sf_command (sndfile, SFC_GET_LOOP_INFO, &loop_info, sizeof (loop_info)) ;
		cmd_sink += loop_info.bpm ;
	} ;

	if (ctl [0] & 4)
	{	/* Cue points. */
		memset (&cues, 0, sizeof (cues)) ;
		cues.cue_count = 100 ;
		sf_command (sndfile, SFC_GET_CUE_COUNT, &count, sizeof (count)) ;
		if (count > 0)
			sf_command (sndfile, SFC_GET_CUE, &cues, sizeof (cues)) ;
		cmd_sink += (double) cues.cue_count ;

		/* Broadcast (bext) metadata: fixed 256-byte description field. */
		bext = (SF_BROADCAST_INFO *) calloc (1, sizeof (SF_BROADCAST_INFO)) ;
		if (bext != NULL)
		{	bext->version = 1 ;
			if (sf_command (sndfile, SFC_GET_BROADCAST_INFO, bext, sizeof (SF_BROADCAST_INFO)))
			{	bext->description [0] ^= 0x21 ;
				bext->coding_history_size = 0 ;
				sf_command (sndfile, SFC_SET_BROADCAST_INFO, bext, sizeof (SF_BROADCAST_INFO)) ;
			} ;
			free (bext) ;
		} ;

		/* Cart (cart chunk) metadata, bounded tag text. */
		cart = (SF_CART_INFO *) calloc (1, sizeof (SF_CART_INFO)) ;
		if (cart != NULL)
		{	memcpy (cart->version, "0101", 4) ;
			memcpy (cart->title, "cmd_fuzzer", 10) ;
			cart->tag_text_size = ctl [2] & 0x3f ;
			for (k = 0 ; k < (int) cart->tag_text_size ; k++)
				cart->tag_text [k] = (char) (k * 31 + ctl [3]) ;
			sf_command (sndfile, SFC_SET_CART_INFO, cart, sizeof (SF_CART_INFO)) ;
			free (cart) ;
		} ;
	} ;

	if (ctl [0] & 8)
	{	/* Format tables. */
		sf_command (sndfile, SFC_GET_SIMPLE_FORMAT_COUNT, &simple_count, sizeof (simple_count)) ;
		if (simple_count > 0)
		{	memset (&fmt_info, 0, sizeof (fmt_info)) ;
			fmt_info.format = (ctl [2] >> 3) % simple_count ;
			sf_command (sndfile, SFC_GET_SIMPLE_FORMAT, &fmt_info, sizeof (fmt_info)) ;
			cmd_sink += (double) fmt_info.format ;
		} ;
	} ;
} /* cmd_drive_commands */


extern "C" int LLVMFuzzerTestOneInput (const uint8_t * data, size_t size)
{	SF_VIRTUAL_IO vio ;
	CMD_VIO cmd_vio ;
	SF_INFO sfinfo ;
	SNDFILE *sndfile = NULL ;
	short *write_buffer = NULL ;
	float *read_buffer = NULL ;
	const uint8_t * file_data ;
	size_t file_size ;
	sf_count_t seek_to, wrote ;
	int write_chunk, k, mode, r ;
	uint8_t ctl [4] ;

	if (size < 8)
		return 0 ;

	/* Layout: [file bytes ...][4 control bytes]. */
	file_size = size - 4 ;
	file_data = data ;
	memcpy (ctl, data + file_size, 4) ;

	cmd_vio.data = (unsigned char *) malloc (file_size) ;
	write_buffer = (short *) malloc (sizeof (short) * 256 * 2) ;
	/* read_buffer is allocated after sf_open: sf_readf_float() zeroes
	** len * <the opened file's channels> floats, so the buffer must be
	** sized from sfinfo.channels, not a fixed 2-channel guess. */
	if (cmd_vio.data == NULL || write_buffer == NULL)
		goto EXIT_LABEL ;

	memcpy (cmd_vio.data, file_data, file_size) ;
	cmd_vio.length = file_size ;
	cmd_vio.offset = 0 ;

	vio.get_filelen = cmd_get_filelen ;
	vio.seek = cmd_seek ;
	vio.read = cmd_read ;
	vio.write = cmd_write ;
	vio.tell = cmd_tell ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	/* RDWR first: exercises the in-place header-update machinery. Files
	** that cannot be updated (headerless RAW, corrupt headers) fall back
	** to a read-only handle; the command driver is safe on both. */
	mode = SFM_RDWR ;
	sndfile = sf_open_virtual (&vio, mode, &sfinfo, &cmd_vio) ;
	if (sndfile == NULL)
	{	memset (&sfinfo, 0, sizeof (sfinfo)) ;
		cmd_vio.offset = 0 ;
		mode = SFM_READ ;
		sndfile = sf_open_virtual (&vio, mode, &sfinfo, &cmd_vio) ;
	} ;

	if (sndfile == NULL)
		goto EXIT_LABEL ;

	cmd_drive_commands (sndfile, ctl, sfinfo.frames, sfinfo.channels) ;

	if (mode == SFM_RDWR && sfinfo.channels >= 1 && sfinfo.channels <= 2 && sfinfo.frames > 0)
	{	static const int chunk_tab [4] = { 1, 64, 128, 256 } ;
		const int channels = sfinfo.channels ;

		write_chunk = chunk_tab [ctl [3] & 3] ;
		seek_to = (sf_count_t) ((ctl [3] >> 2) & 0x3f) * sfinfo.frames / 64 ;

		if (sf_seek (sndfile, seek_to, SEEK_SET) >= 0)
		{	for (k = 0 ; k < write_chunk * channels ; k++)
				write_buffer [k] = (short) ((k * 257 + ctl [2]) & 0x7fff) ;

			wrote = sf_writef_short (sndfile, write_buffer, write_chunk) ;
			if (wrote > 0)
				sf_write_sync (sndfile) ;

			sf_command (sndfile, SFC_UPDATE_HEADER_NOW, NULL, 0) ;
		} ;
	} ;

	/* A bounded float read pass: conversion + dither-on-read paths.
	** Buffer sized from the file's actual channel count (bounded). */
	if (sfinfo.channels >= 1 && sfinfo.channels <= 256)
	{	read_buffer = (float *) malloc (sizeof (float) * 256 * sfinfo.channels) ;
		if (read_buffer != NULL)
		{	sf_seek (sndfile, 0, SEEK_SET) ;
			r = 16 ;
			while (r-- > 0 && sf_readf_float (sndfile, read_buffer, 256) > 0)
				cmd_sink += read_buffer [0] ;
			} ;
		} ;

EXIT_LABEL :

	if (sndfile != NULL)
		sf_close (sndfile) ;

	free (cmd_vio.data) ;
	free (write_buffer) ;
	free (read_buffer) ;

	return 0 ;
} /* LLVMFuzzerTestOneInput */

