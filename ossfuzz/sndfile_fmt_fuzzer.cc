#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sndfile.h>
#include <inttypes.h>

#include "sndfile_fuzz_header.h"

// sndfile_fuzzer / sndfile_alt_fuzzer leave SF_INFO zeroed, so sf_open_virtual
// only reaches the self-describing (header-bearing) formats. The headerless RAW
// sub-formats and SD2 are never decoded. This harness picks an explicit format
// from the first byte so the RAW codec readers (vox_adpcm.c, alaw.c, ulaw.c,
// gsm610.c, dwvw.c), pcm.c/raw.c and sd2.c get exercised.

static const int formats[] = {
    SF_FORMAT_RAW | SF_FORMAT_VOX_ADPCM,
    SF_FORMAT_RAW | SF_FORMAT_PCM_16,
    SF_FORMAT_RAW | SF_FORMAT_PCM_U8,
    SF_FORMAT_RAW | SF_FORMAT_ALAW,
    SF_FORMAT_RAW | SF_FORMAT_ULAW,
    SF_FORMAT_RAW | SF_FORMAT_GSM610,
    SF_FORMAT_RAW | SF_FORMAT_DWVW_16,
    SF_FORMAT_SD2,
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{   VIO_DATA vio_data ;
    SF_VIRTUAL_IO vio ;
    SF_INFO sndfile_info ;
    SNDFILE *sndfile = NULL ;
    short *read_buffer = NULL ;

    if (size < 1)
      return 0 ;

    int fmt = formats[data[0] % (sizeof(formats) / sizeof(formats[0]))] ;
    data += 1 ; size -= 1 ;

    vio.get_filelen = vfget_filelen ;
    vio.seek = vfseek ;
    vio.read = vfread ;
    vio.write = vfwrite ;
    vio.tell = vftell ;

    vio_data.data = data ;
    vio_data.length = size ;
    vio_data.offset = 0 ;

    memset(&sndfile_info, 0, sizeof(SF_INFO)) ;
    sndfile_info.format = fmt ;
    sndfile_info.channels = 1 ;
    sndfile_info.samplerate = 8000 ;

    sndfile = sf_open_virtual(&vio, SFM_READ, &sndfile_info, &vio_data) ;
    if (sndfile == NULL)
      return 0 ;

    if (sndfile_info.channels < 1 || sndfile_info.channels > 1024)
      goto EXIT_LABEL ;

    read_buffer = (short *) malloc(sizeof(short) * sndfile_info.channels) ;
    if (read_buffer == NULL)
      goto EXIT_LABEL ;

    // Cap the read loop: some headerless RAW sub-formats can otherwise decode
    // far more frames than the input bytes (e.g. via back-seeks), which reads
    // as a fuzzing timeout rather than a finding.
    for (long i = 0 ; i < (1L << 20) ; i++)
    {
      if (sf_readf_short(sndfile, read_buffer, 1) == 0)
        break ;
    }

EXIT_LABEL:

    if (sndfile != NULL)
      sf_close(sndfile) ;

    free(read_buffer) ;

    return 0 ;
}
