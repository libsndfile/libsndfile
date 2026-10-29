#!/usr/bin/env python3
"""Generate the seed corpus for sndfile_cmd_fuzzer.

The command-surface harness splits its input into [file bytes][4 control
bytes].  These seeds are minimal but fully valid containers (the library
must get past header parsing before the command machinery runs), each
appended with a control word exercising a different command group:

    0x01 0x04 0x00 0x0F  -> peak calculations + header-update modes
    0x02 0x0C 0x20 0x14  -> instrument/loop metadata, dither, update-now
    0x04 0x0C 0x41 0x2A  -> cue/bext/cart metadata paths
    0x08 0x04 0x08 0x30  -> format-table sweeps
    0x0F 0x35 0x6B 0x3F  -> everything, auto header updates, 48 kHz-style

Containers: WAV, RIFX (big-endian WAV), AIFF, AU, CAF, NIST, W64, SD2.
Run from ossfuzz.sh:  python3 sndfile_cmd_seeds.py $OUT
Writes $OUT/sndfile_cmd_fuzzer_seed_corpus.zip (no zip binary dependency).
"""

import os
import struct
import sys
import zipfile


def extended_float(rate):
    """AIFF 80-bit extended float for a sample rate (valid for rate < 2^31).

    value = mantissa(1.frac, 64 explicit bits) * 2 ** (exp - 16383);
    for rate = 8000 this yields the canonical 40 0B FA 00 ... bytes.
    """
    x = rate.bit_length() - 1
    e = 16383 + x
    hi = (rate << (31 - x)) & 0xFFFFFFFF
    return struct.pack(">HIH", e, hi, 0)


def wav_pcm16(rate=8000, channels=1, frames=8):
    data = b"\x00\x01\x00\xff\x12\x34\xfe\xdc" * frames
    byte_rate = rate * channels * 2
    fmt = struct.pack(
        "<HHIIHH", 1, channels, rate, byte_rate, channels * 2, 16
    )
    body = b"WAVE" + b"fmt " + struct.pack("<I", len(fmt)) + fmt
    body += b"data" + struct.pack("<I", len(data)) + data
    return b"RIFF" + struct.pack("<I", len(body)) + body


def wav_rifx_pcm16(rate=8000, channels=1, frames=8):
    data = b"\x00\x01\x00\xff\x12\x34\xfe\xdc" * frames
    fmt = struct.pack(
        ">HHIIHH", 1, channels, rate, rate * channels * 2, channels * 2, 16
    )
    body = b"WAVE" + b"fmt " + struct.pack(">I", len(fmt)) + fmt
    body += b"data" + struct.pack(">I", len(data)) + data
    return b"RIFX" + struct.pack(">I", len(body)) + body


def aiff_pcm16(rate=8000, channels=1, frames=8):
    data = b"\x00\x01\x00\xff\x12\x34\xfe\xdc" * frames
    comm = struct.pack(">hIh", channels, frames, 16) + extended_float(rate)
    ssnd = struct.pack(">II", 0, 0) + data
    body = (
        b"AIFF"
        + b"COMM" + struct.pack(">I", len(comm)) + comm
        + b"SSND" + struct.pack(">I", len(ssnd)) + ssnd
    )
    return b"FORM" + struct.pack(">I", len(body)) + body


def au_pcm16(rate=8000):
    data = b"\x00\x01\x00\xff\x12\x34\xfe\xdc" * 8
    hdr = struct.pack(">IIIIII", 0x2E736E64, 24, len(data), 3, rate, 1)
    return hdr + data


def caf_pcm16(rate=8000, channels=1, frames=8):
    data = b"\x00\x01\x00\xff\x12\x34\xfe\xdc" * frames
    # desc chunk: mSampleRate double + 7 uint32 (formatID 'lpcm', flags,
    # bytes/packet, frames/packet, bytes/frame, channels, bits).
    desc = struct.pack(">d7I", float(rate), 0x6C70636D, 0,
                       2 * channels, 1, 2 * channels, channels, 16)
    body = b"caff" + struct.pack(">HH", 1, 0)
    body += b"desc" + struct.pack(">Q", len(desc)) + desc
    body += b"data" + struct.pack(">QQ", len(data), 0) + data
    return body


def nist_pcm16(rate=8000, channels=1):
    data = b"\x00\x01\x00\xff\x12\x34\xfe\xdc" * 8
    hdr = b"NIST_1A\n   1024\n"
    hdr += b"channel_count i %d\n" % channels
    hdr += b"sample_n_bytes i 2\nsample_rate i %d\n" % rate
    hdr += b"sample_byte_align i 2\nsample_coding PCM\nsample_sig integer\n"
    hdr += b"end_head\n"
    hdr += b"\x00" * (1024 - len(hdr))
    return hdr + data


# WAV/RIFX/AIFF/AU/CAF/NIST only: W64 (16-byte GUID walkers) and SD2
# (resource-fork layout) seeds need exact binary layouts that are easy to
# get subtly wrong; mutations of the valid seeds reach those parsers.
CONTAINERS = [
    ("wav", wav_pcm16()),
    ("rifx", wav_rifx_pcm16()),
    ("aiff", aiff_pcm16()),
    ("au", au_pcm16()),
    ("caf", caf_pcm16()),
    ("nist", nist_pcm16()),
]


CONTROLS = [
    bytes([0x01, 0x04, 0x00, 0x0F]),
    bytes([0x02, 0x0C, 0x20, 0x14]),
    bytes([0x04, 0x0C, 0x41, 0x2A]),
    bytes([0x08, 0x04, 0x08, 0x30]),
    bytes([0x0F, 0x35, 0x6B, 0x3F]),
]


def main():
    out_dir = sys.argv[1] if len(sys.argv) > 1 else "."
    zip_path = os.path.join(out_dir, "sndfile_cmd_fuzzer_seed_corpus.zip")

    entries = []
    for name, blob in CONTAINERS:
        for i, ctl in enumerate(CONTROLS):
            entries.append(("cmd_seed_%s_ctl%d" % (name, i), blob + ctl))

    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zf:
        for name, blob in entries:
            zf.writestr(name, blob)

    print("sndfile_cmd_seeds: wrote %d seeds to %s" % (len(entries), zip_path))


if __name__ == "__main__":
    main()
