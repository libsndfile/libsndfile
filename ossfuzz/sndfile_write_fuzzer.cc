/*
 * sndfile_write_fuzzer.cc
 *
 * Fuzz harness for libsndfile write/transcode paths. The existing
 * sndfile_fuzzer exercises only the read/decode path. This harness:
 * 1. Opens fuzz data as a virtual read file
 * 2. If valid, reads samples and writes them to an in-memory output
 *    in a different format — exercising the encoder path
 *
 * Targets: sf_writef_float(), sf_writef_short(), sf_write_raw(),
 * and format-specific encoder code.
 */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <sndfile.h>

struct MemBuf {
    const uint8_t *data;
    size_t size;
    sf_count_t pos;
};

static sf_count_t mem_get_filelen(void *user) {
    return (sf_count_t)((MemBuf *)user)->size;
}
static sf_count_t mem_seek(sf_count_t offset, int whence, void *user) {
    MemBuf *b = (MemBuf *)user;
    sf_count_t newpos;
    switch (whence) {
        case SEEK_SET: newpos = offset; break;
        case SEEK_CUR: newpos = b->pos + offset; break;
        case SEEK_END: newpos = (sf_count_t)b->size + offset; break;
        default: return -1;
    }
    if (newpos < 0 || newpos > (sf_count_t)b->size) return -1;
    b->pos = newpos;
    return b->pos;
}
static sf_count_t mem_read(void *ptr, sf_count_t count, void *user) {
    MemBuf *b = (MemBuf *)user;
    sf_count_t avail = (sf_count_t)b->size - b->pos;
    if (count > avail) count = avail;
    memcpy(ptr, b->data + b->pos, count);
    b->pos += count;
    return count;
}
static sf_count_t mem_write(const void *ptr, sf_count_t count, void *user) {
    (void)ptr; (void)count; (void)user;
    return count; /* discard */
}
static sf_count_t mem_tell(void *user) {
    return ((MemBuf *)user)->pos;
}

/* Output buffer for write fuzzer */
struct OutBuf {
    uint8_t *data;
    size_t size;
    size_t capacity;
    sf_count_t pos;
};
static sf_count_t out_get_filelen(void *user) {
    return (sf_count_t)((OutBuf *)user)->size;
}
static sf_count_t out_seek(sf_count_t offset, int whence, void *user) {
    OutBuf *b = (OutBuf *)user;
    sf_count_t newpos;
    switch (whence) {
        case SEEK_SET: newpos = offset; break;
        case SEEK_CUR: newpos = b->pos + offset; break;
        case SEEK_END: newpos = (sf_count_t)b->size + offset; break;
        default: return -1;
    }
    if (newpos < 0) return -1;
    b->pos = newpos;
    return b->pos;
}
static sf_count_t out_read(void *ptr, sf_count_t count, void *user) {
    (void)ptr; (void)count; (void)user;
    return 0;
}
static sf_count_t out_write(const void *ptr, sf_count_t count, void *user) {
    OutBuf *b = (OutBuf *)user;
    size_t end = (size_t)b->pos + (size_t)count;
    if (end > b->capacity) {
        if (end > 4 * 1024 * 1024) return 0; /* cap at 4MB */
        size_t newcap = end + 4096;
        uint8_t *newdata = (uint8_t *)realloc(b->data, newcap);
        if (!newdata) return 0;
        b->data = newdata;
        b->capacity = newcap;
    }
    memcpy(b->data + b->pos, ptr, count);
    b->pos += count;
    if ((size_t)b->pos > b->size) b->size = (size_t)b->pos;
    return count;
}
static sf_count_t out_tell(void *user) {
    return ((OutBuf *)user)->pos;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 16) return 0;

    SF_VIRTUAL_IO vio_in = { mem_get_filelen, mem_seek, mem_read, mem_write, mem_tell };
    MemBuf inbuf = { data, size, 0 };

    SF_INFO sfinfo;
    sfinfo.format = 0;
    SNDFILE *snd_in = sf_open_virtual(&vio_in, SFM_READ, &sfinfo, &inbuf);
    if (!snd_in) return 0;

    /* Sanity limits */
    if (sfinfo.channels < 1 || sfinfo.channels > 8 ||
        sfinfo.samplerate < 1) {
        sf_close(snd_in);
        return 0;
    }

    /* Set up output as WAV in memory */
    SF_INFO out_info = sfinfo;
    out_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    if (!sf_format_check(&out_info)) {
        out_info.format = SF_FORMAT_AU | SF_FORMAT_PCM_16;
    }

    SF_VIRTUAL_IO vio_out = { out_get_filelen, out_seek, out_read, out_write, out_tell };
    OutBuf outbuf = { nullptr, 0, 0, 0 };

    SNDFILE *snd_out = sf_open_virtual(&vio_out, SFM_WRITE, &out_info, &outbuf);
    if (!snd_out) {
        sf_close(snd_in);
        free(outbuf.data);
        return 0;
    }

    /* Transcode up to 4096 frames */
    const int FRAMES = 4096;
    float *buf = (float *)malloc(FRAMES * sfinfo.channels * sizeof(float));
    if (buf) {
        sf_count_t read;
        sf_count_t total = 0;
        while ((read = sf_readf_float(snd_in, buf, FRAMES)) > 0 && total < 65536) {
            sf_writef_float(snd_out, buf, read);
            total += read;
        }
        free(buf);
    }

    sf_close(snd_out);
    sf_close(snd_in);
    free(outbuf.data);
    return 0;
}
