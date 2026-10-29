/*
** sndfile_rw_fuzzer.cc -- write->read round-trip harness for libsndfile.
**
** Threat model
** ------------
** Untrusted input: the byte stream (stands in for attacker-influenced audio
** data and conversion parameters an application feeds the library).
** Trust boundary: the public sf_open_virtual()/sf_writef_double() API on a
** virtual in-memory file.  The attacker controls *which* container/codec
** combination is written, the channel count, frame count and the sample
** values; every combination is validated against sf_format_check() so the
** documented API contract is honoured (no false positives).
**
** Rationale
** ---------
** The production read fuzzers (sndfile_fuzzer, sndfile_alt_fuzzer and the
** newly merged sndfile_fmt_fuzzer) only ever *decode*.  The whole write
** direction -- header writers, codec encoders (pcm.c, alac_encoder.c,
** gsm_encode.c, g72x.c, ima/ms/nms/vox adpcm encode, dwvw, alaw/ulaw), the
** float/double conversion + clip + dither paths (double64.c, float32.c,
** dither.c) and the endswap paths -- is dark in every production coverage
** report.  This harness writes fuzz-selected but structurally valid audio,
** closes, then re-opens and re-reads the produced bytes, so every encoder
** and header writer is exercised and immediately decoded again.
**
** The input is a compact control stream (row index + flag byte + frame
** count + int16 sample stream), not a container file: no dictionary can
** help a table-driven selector, so this harness ships a structured seed
** corpus (sndfile_rw_seeds.py) covering the whole row table instead.
**
** Expected bug classes: encoder-side OOB writes, integer overflow in header
** size arithmetic, dither/clipping arithmetic (UBSan), header-rewrite
** errors, leaks on codec error paths.
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>
#include <inttypes.h>

/* Container/subformat rows curated from sf_format_check() (src/sndfile.c).
** Only codec/container pairs compiled into the OSS-Fuzz build are listed:
** the external-libs modules (FLAC/Ogg/Vorbis/Opus/MPEG) are not built there.
*/
typedef struct
{	int major ;
	int sub ;
	int mono ;		/* codec is mono-only */
	int rate_mode ;		/* 1: fixed 8000 Hz, 2: fixed 32000 Hz (SDS) */
	int endian_flex ;	/* SF_ENDIAN_BIG flip allowed (WAV -> RIFX, AIFF) */
	int peak_ok ;		/* SFC_SET_ADD_PEAK_CHUNK meaningful */
} RW_ROW ;

static const RW_ROW rw_rows [] = {
	/* WAV */
	{	SF_FORMAT_WAV,	SF_FORMAT_PCM_U8,	0, 0, 1, 1 },
	{	SF_FORMAT_WAV,	SF_FORMAT_PCM_16,	0, 0, 1, 1 },
	{	SF_FORMAT_WAV,	SF_FORMAT_PCM_24,	0, 0, 1, 1 },
	{	SF_FORMAT_WAV,	SF_FORMAT_PCM_32,	0, 0, 1, 1 },
	{	SF_FORMAT_WAV,	SF_FORMAT_FLOAT,	0, 0, 1, 1 },
	{	SF_FORMAT_WAV,	SF_FORMAT_DOUBLE,	0, 0, 1, 1 },
	{	SF_FORMAT_WAV,	SF_FORMAT_ALAW,		0, 0, 1, 1 },
	{	SF_FORMAT_WAV,	SF_FORMAT_ULAW,		0, 0, 1, 1 },
	{	SF_FORMAT_WAV,	SF_FORMAT_IMA_ADPCM,	0, 0, 1, 0 },
	{	SF_FORMAT_WAV,	SF_FORMAT_MS_ADPCM,	0, 0, 1, 0 },
	{	SF_FORMAT_WAV,	SF_FORMAT_GSM610,	1, 1, 1, 0 },
	{	SF_FORMAT_WAV,	SF_FORMAT_G721_32,	1, 1, 1, 0 },
	{	SF_FORMAT_WAV,	SF_FORMAT_NMS_ADPCM_16,	1, 1, 1, 0 },
	{	SF_FORMAT_WAV,	SF_FORMAT_NMS_ADPCM_24,	1, 1, 1, 0 },
	{	SF_FORMAT_WAV,	SF_FORMAT_NMS_ADPCM_32,	1, 1, 1, 0 },
	/* WAVEX */
	{	SF_FORMAT_WAVEX,	SF_FORMAT_PCM_16,	0, 0, 0, 1 },
	{	SF_FORMAT_WAVEX,	SF_FORMAT_PCM_24,	0, 0, 0, 1 },
	{	SF_FORMAT_WAVEX,	SF_FORMAT_FLOAT,	0, 0, 0, 1 },
	{	SF_FORMAT_WAVEX,	SF_FORMAT_ULAW,		0, 0, 0, 1 },
	/* AIFF */
	{	SF_FORMAT_AIFF,	SF_FORMAT_PCM_16,	0, 0, 1, 1 },
	{	SF_FORMAT_AIFF,	SF_FORMAT_PCM_24,	0, 0, 1, 1 },
	{	SF_FORMAT_AIFF,	SF_FORMAT_PCM_32,	0, 0, 1, 1 },
	{	SF_FORMAT_AIFF,	SF_FORMAT_PCM_S8,	0, 0, 0, 1 },
	{	SF_FORMAT_AIFF,	SF_FORMAT_FLOAT,	0, 0, 0, 1 },
	{	SF_FORMAT_AIFF,	SF_FORMAT_DOUBLE,	0, 0, 0, 1 },
	{	SF_FORMAT_AIFF,	SF_FORMAT_ULAW,		0, 0, 0, 1 },
	{	SF_FORMAT_AIFF,	SF_FORMAT_ALAW,		0, 0, 0, 1 },
	{	SF_FORMAT_AIFF,	SF_FORMAT_IMA_ADPCM,	0, 0, 0, 0 },
	/* AIFF|DWVW_12 is quarantined: upstream aiff_open() never calls
	** dwvw_init() for SF_FORMAT_DWVW_12 (unlike DWVW_16/24), so a write
	** handle has NULL codec write pointers and sf_writef_*() dereferences
	** NULL (finding R-1, reproducer rw-crash-input.bin). The DWVW_12
	** encoder is still covered through the RAW container, whose init
	** (src/raw.c) does wire dwvw_init (psf, 12). */
	{	SF_FORMAT_RAW,	SF_FORMAT_DWVW_12,	1, 1, 0, 0 },
	{	SF_FORMAT_AIFF,	SF_FORMAT_DWVW_16,	1, 1, 0, 0 },
	{	SF_FORMAT_AIFF,	SF_FORMAT_DWVW_24,	1, 1, 0, 0 },
	{	SF_FORMAT_AIFF,	SF_FORMAT_GSM610,	1, 1, 0, 0 },
	/* AU */
	{	SF_FORMAT_AU,	SF_FORMAT_PCM_S8,	0, 0, 0, 0 },
	{	SF_FORMAT_AU,	SF_FORMAT_PCM_16,	0, 0, 0, 0 },
	{	SF_FORMAT_AU,	SF_FORMAT_PCM_24,	0, 0, 0, 0 },
	{	SF_FORMAT_AU,	SF_FORMAT_PCM_32,	0, 0, 0, 0 },
	{	SF_FORMAT_AU,	SF_FORMAT_FLOAT,	0, 0, 0, 0 },
	{	SF_FORMAT_AU,	SF_FORMAT_ULAW,		0, 0, 0, 0 },
	{	SF_FORMAT_AU,	SF_FORMAT_ALAW,		0, 0, 0, 0 },
	{	SF_FORMAT_AU,	SF_FORMAT_G721_32,	1, 1, 0, 0 },
	{	SF_FORMAT_AU,	SF_FORMAT_G723_24,	1, 1, 0, 0 },
	{	SF_FORMAT_AU,	SF_FORMAT_G723_40,	1, 1, 0, 0 },
	/* CAF */
	{	SF_FORMAT_CAF,	SF_FORMAT_PCM_16,	0, 0, 0, 1 },
	{	SF_FORMAT_CAF,	SF_FORMAT_PCM_24,	0, 0, 0, 1 },
	{	SF_FORMAT_CAF,	SF_FORMAT_ALAC_16,	0, 0, 0, 0 },
	{	SF_FORMAT_CAF,	SF_FORMAT_ALAC_24,	0, 0, 0, 0 },
	{	SF_FORMAT_CAF,	SF_FORMAT_ALAC_32,	0, 0, 0, 0 },
	{	SF_FORMAT_CAF,	SF_FORMAT_ULAW,		0, 0, 0, 1 },
	/* RAW (headerless: encode-only exercise) */
	{	SF_FORMAT_RAW,	SF_FORMAT_PCM_16,	0, 0, 0, 0 },
	{	SF_FORMAT_RAW,	SF_FORMAT_PCM_32,	0, 0, 0, 0 },
	{	SF_FORMAT_RAW,	SF_FORMAT_ULAW,		0, 0, 0, 0 },
	{	SF_FORMAT_RAW,	SF_FORMAT_VOX_ADPCM,	1, 1, 0, 0 },
	{	SF_FORMAT_RAW,	SF_FORMAT_DWVW_16,	1, 1, 0, 0 },
	{	SF_FORMAT_RAW,	SF_FORMAT_NMS_ADPCM_32,	1, 1, 0, 0 },
	{	SF_FORMAT_RAW,	SF_FORMAT_GSM610,	1, 1, 0, 0 },
	/* Remaining containers */
	{	SF_FORMAT_PAF,	SF_FORMAT_PCM_16,	0, 0, 0, 0 },
	{	SF_FORMAT_SVX,	SF_FORMAT_PCM_16,	1, 0, 0, 0 },
	{	SF_FORMAT_NIST,	SF_FORMAT_PCM_16,	0, 0, 0, 0 },
	{	SF_FORMAT_NIST,	SF_FORMAT_ULAW,		0, 0, 0, 0 },
	{	SF_FORMAT_IRCAM,	SF_FORMAT_PCM_16,	0, 0, 0, 0 },
	{	SF_FORMAT_VOC,	SF_FORMAT_PCM_16,	0, 0, 0, 0 },
	{	SF_FORMAT_VOC,	SF_FORMAT_ULAW,		0, 0, 0, 0 },
	{	SF_FORMAT_W64,	SF_FORMAT_PCM_16,	0, 0, 0, 1 },
	{	SF_FORMAT_W64,	SF_FORMAT_IMA_ADPCM,	0, 0, 0, 0 },
	{	SF_FORMAT_W64,	SF_FORMAT_MS_ADPCM,	0, 0, 0, 0 },
	{	SF_FORMAT_W64,	SF_FORMAT_GSM610,	1, 1, 0, 0 },
	{	SF_FORMAT_MAT4,	SF_FORMAT_PCM_16,	0, 0, 0, 0 },
	{	SF_FORMAT_MAT4,	SF_FORMAT_DOUBLE,	0, 0, 0, 0 },
	{	SF_FORMAT_MAT5,	SF_FORMAT_PCM_16,	0, 0, 0, 0 },
	{	SF_FORMAT_PVF,	SF_FORMAT_PCM_16,	0, 0, 0, 0 },
	{	SF_FORMAT_XI,	SF_FORMAT_DPCM_16,	1, 0, 0, 0 },
	{	SF_FORMAT_XI,	SF_FORMAT_DPCM_8,	1, 0, 0, 0 },
	{	SF_FORMAT_HTK,	SF_FORMAT_PCM_16,	1, 0, 0, 0 },
	{	SF_FORMAT_SDS,	SF_FORMAT_PCM_16,	1, 2, 0, 0 },
	{	SF_FORMAT_AVR,	SF_FORMAT_PCM_16,	0, 0, 0, 0 },
	{	SF_FORMAT_SD2,	SF_FORMAT_PCM_16,	0, 0, 0, 0 },
	{	SF_FORMAT_WVE,	SF_FORMAT_ALAW,		1, 1, 0, 0 },
	{	SF_FORMAT_MPC2K,	SF_FORMAT_PCM_16,	0, 0, 0, 0 },
	{	SF_FORMAT_RF64,	SF_FORMAT_PCM_16,	0, 0, 0, 1 },
} ;

#define RW_ROW_COUNT	((int) (sizeof (rw_rows) / sizeof (rw_rows [0])))
#define RW_MAX_FRAMES	2048
#define RW_CHUNK	256


/* In-memory virtual file with a real write direction (the shared
** sndfile_fuzz_header.h vio cannot write; this harness needs it). */
typedef struct
{	unsigned char *data ;
	sf_count_t length ;
	sf_count_t capacity ;
	sf_count_t offset ;
} RW_VIO ;

static void rw_vio_init (RW_VIO *vio)
{	vio->data = NULL ;
	vio->length = 0 ;
	vio->capacity = 0 ;
	vio->offset = 0 ;
} /* rw_vio_init */

static sf_count_t rw_get_filelen (void *user_data)
{	RW_VIO *vf = (RW_VIO *) user_data ;
	return vf->length ;
} /* rw_get_filelen */

static sf_count_t rw_seek (sf_count_t offset, int whence, void *user_data)
{	RW_VIO *vf = (RW_VIO *) user_data ;

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
} /* rw_seek */

static sf_count_t rw_read (void *ptr, sf_count_t count, void *user_data)
{	RW_VIO *vf = (RW_VIO *) user_data ;

	if (vf->offset + count > vf->length)
		count = vf->length - vf->offset ;
	if (count < 0)
		return 0 ;

	memcpy (ptr, vf->data + vf->offset, count) ;
	vf->offset += count ;

	return count ;
} /* rw_read */

static sf_count_t rw_write (const void *ptr, sf_count_t count, void *user_data)
{	RW_VIO *vf = (RW_VIO *) user_data ;

	if (vf->offset + count > vf->capacity)
	{	sf_count_t new_capacity = vf->capacity ? vf->capacity : 64 * 1024 ;
		unsigned char *new_data ;

		while (vf->offset + count > new_capacity)
			new_capacity *= 2 ;

		new_data = (unsigned char *) realloc (vf->data, new_capacity) ;
		if (new_data == NULL)
			return 0 ;

		vf->data = new_data ;
		vf->capacity = new_capacity ;
	} ;

	memcpy (vf->data + vf->offset, ptr, count) ;
	vf->offset += count ;
	if (vf->offset > vf->length)
		vf->length = vf->offset ;

	return count ;
} /* rw_write */

static sf_count_t rw_tell (void *user_data)
{	RW_VIO *vf = (RW_VIO *) user_data ;
	return vf->offset ;
} /* rw_tell */

static volatile double rw_sink = 0.0 ;


extern "C" int LLVMFuzzerTestOneInput (const uint8_t * data, size_t size)
{	SF_VIRTUAL_IO vio ;
	RW_VIO rw_vio ;
	SF_INFO sfinfo ;
	SNDFILE *sndfile = NULL ;
	double *write_buffer = NULL ;
	short *read_buffer = NULL ;
	const uint8_t * samples_in ;
	size_t samples_left ;
	int row_index, channels, samplerate, frames, format, dither_type ;
	int endian_flip, want_peak, want_broadcast, want_update ;
	int wrote_frames, r ;
	sf_count_t written = 0 ;

	if (size < 4)
		return 0 ;

	/* Control stream layout (mirrored by ossfuzz/sndfile_rw_seeds.py):
	** byte 0     : row index
	** byte 1     : bit 0 endian flip, bits 1-2 dither type, bit 2 PEAK chunk,
	**              bit 3 broadcast info, bit 4 UPDATE_HEADER_NOW,
	**              bit 5 second channel, bit 7 rate 48000
	** bytes 2-3  : frame count - 1, big endian, modulo RW_MAX_FRAMES
	** remainder  : int16 LE sample stream, cycled to fill frames*channels
	*/
	row_index	= data [0] % RW_ROW_COUNT ;
	endian_flip	= data [1] & 0x01 ;
	dither_type	= (data [1] >> 1) & 0x03 ;
	want_peak	= (data [1] >> 2) & 0x01 ;
	want_broadcast	= (data [1] >> 3) & 0x01 ;
	want_update	= (data [1] >> 4) & 0x01 ;
	channels	= ((data [1] >> 5) & 0x01) + 1 ;
	samplerate	= (data [1] & 0x80) ? 48000 : 44100 ;
	frames		= ((data [2] << 8) | data [3]) % RW_MAX_FRAMES + 1 ;

	samples_in	= data + 4 ;
	samples_left	= size - 4 ;

	{	const RW_ROW *row = &rw_rows [row_index] ;

		format = row->major | row->sub ;
		if (row->rate_mode == 1)
			samplerate = 8000 ;
		else if (row->rate_mode == 2)
			samplerate = 32000 ;	/* SDS */
		if (row->mono)
			channels = 1 ;
		if (row->endian_flex && endian_flip)
			format |= SF_ENDIAN_BIG ;
		want_peak = want_peak & row->peak_ok ;
		want_broadcast = want_broadcast && (row->major == SF_FORMAT_WAV) ;
	} ;

	write_buffer = (double *) malloc (sizeof (double) * RW_CHUNK * channels) ;
	read_buffer = (short *) malloc (sizeof (short) * RW_CHUNK * channels) ;
	if (write_buffer == NULL || read_buffer == NULL)
		goto EXIT_LABEL ;

	rw_vio_init (&rw_vio) ;

	vio.get_filelen = rw_get_filelen ;
	vio.seek = rw_seek ;
	vio.read = rw_read ;
	vio.write = rw_write ;
	vio.tell = rw_tell ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	sfinfo.format = format ;
	sfinfo.channels = channels ;
	sfinfo.samplerate = samplerate ;

	sndfile = sf_open_virtual (&vio, SFM_WRITE, &sfinfo, &rw_vio) ;
	if (sndfile == NULL)
		goto EXIT_LABEL ;

	if (want_peak)
		sf_command (sndfile, SFC_SET_ADD_PEAK_CHUNK, NULL, SF_TRUE) ;

	if (want_broadcast)
	{	SF_BROADCAST_INFO bext ;
		size_t n ;

		memset (&bext, 0, sizeof (bext)) ;
		bext.version = 1 ;
		n = samples_left < sizeof (bext.description) - 1 ? samples_left : sizeof (bext.description) - 1 ;
		memcpy (bext.description, samples_in, n) ;
		sf_command (sndfile, SFC_SET_BROADCAST_INFO, &bext, sizeof (bext)) ;
	} ;

	/* Dither on write only bites for integer subformats: exactly the dark
	** dither.c / pcm.c conversion code. */
	{	SF_DITHER_INFO dither ;

		memset (&dither, 0, sizeof (dither)) ;
		switch (dither_type)
		{	case 0 : dither.type = SFD_DEFAULT_LEVEL ; break ;
			case 1 : dither.type = SFD_WHITE ; break ;
			case 2 : dither.type = SFD_TRIANGULAR_PDF ; break ;
			default : dither.type = SFD_NO_DITHER ; break ;
			} ;
		dither.level = 1.0 ;
		sf_command (sndfile, SFC_SET_DITHER_ON_WRITE, &dither, sizeof (dither)) ;
	} ;


	/* Fill chunks deterministically from the sample stream, cycling when
	** the input is exhausted; normalised doubles honour the API contract. */
	{	size_t produced = 0 ;
		const size_t total = (size_t) frames * channels ;

		while (produced < total)
		{	size_t chunk = total - produced ;
			size_t i ;

			if (chunk > RW_CHUNK)
				chunk = RW_CHUNK ;

			for (i = 0 ; i < chunk ; i++)
			{	int16_t s ;

				if (samples_left >= 2)
					s = (int16_t) ((samples_in [(produced + i) * 2 % samples_left] << 8) | samples_in [((produced + i) * 2 + 1) % samples_left]) ;
				else
					s = (int16_t) ((produced + i) * 137) ;
				write_buffer [i] = s / 32768.0 ;
			} ;

			wrote_frames = sf_writef_double (sndfile, write_buffer, chunk / channels) ;
			if (wrote_frames <= 0)
				break ;
			written += wrote_frames ;
			produced += (size_t) wrote_frames * channels ;

			if (want_update && produced == (size_t) chunk)
				sf_command (sndfile, SFC_UPDATE_HEADER_NOW, NULL, 0) ;
		} ;
	} ;

	sf_close (sndfile) ;
	sndfile = NULL ;

	if (rw_vio.length < 16)
		goto EXIT_LABEL ;

	/* Second phase: re-open what was just written (fresh header detection
	** of the library's own output) and read it back, with a seek. */
	rw_vio.offset = 0 ;
	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	sndfile = sf_open_virtual (&vio, SFM_READ, &sfinfo, &rw_vio) ;
	if (sndfile == NULL)
		goto EXIT_LABEL ;

	/* sf_readf_short()'s contract: the caller's buffer holds
	** len * <the opened file's channels> items. The re-detected channel
	** count can differ from the write-phase one, so size for what the
	** file actually reports (bounded; reads are skipped otherwise). */
	if (sfinfo.channels >= 1 && sfinfo.channels <= 256)
	{	short *new_rb = (short *) realloc (read_buffer, sizeof (short) * RW_CHUNK * sfinfo.channels) ;

		if (new_rb != NULL)
		{	read_buffer = new_rb ;

			sf_seek (sndfile, written / 3, SEEK_SET) ;
			while ((r = sf_readf_short (sndfile, read_buffer, RW_CHUNK)) > 0)
				rw_sink += read_buffer [0] ;

			sf_seek (sndfile, 0, SEEK_SET) ;
			while ((r = sf_readf_short (sndfile, read_buffer, RW_CHUNK)) > 0)
				rw_sink -= read_buffer [sfinfo.channels - 1] ;
		} ;
	} ;

EXIT_LABEL :

	if (sndfile != NULL)
		sf_close (sndfile) ;

	free (rw_vio.data) ;
	free (write_buffer) ;
	free (read_buffer) ;

	return 0 ;
} /* LLVMFuzzerTestOneInput */

