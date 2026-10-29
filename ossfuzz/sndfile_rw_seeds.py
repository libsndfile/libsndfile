#!/usr/bin/env python3
"""Generate the structured seed corpus for sndfile_rw_fuzzer.

The round-trip harness (sndfile_rw_fuzzer.cc) consumes a compact control
stream, not a container file:

    byte 0    : row index into the harness's container/codec table
    byte 1    : flag bits (endian flip, dither type, PEAK chunk, broadcast
                info, UPDATE_HEADER_NOW, second channel, 48 kHz rate)
    bytes 2-3 : frame count - 1, big endian (modulo 2048)
    remainder : int16 LE sample stream

Random mutation handles such a table poorly (one byte selects the whole
codec path), so the corpus covers every row at least twice: once with a
frugal mono/no-options layout and once with the option bits set (dither,
PEAK/broadcast metadata, stereo, RIFX/big-endian flip where allowed).

Run from ossfuzz.sh:  python3 sndfile_rw_seeds.py $OUT
Writes $OUT/sndfile_rw_fuzzer_seed_corpus.zip directly (no zip binary
dependency).
"""

import os
import struct
import sys
import zipfile

# Must match rw_rows[] in sndfile_rw_fuzzer.cc.
ROW_COUNT = 84


def sample_stream(n):
    """A short int16 LE stream with extremes, squares and a ramp."""
    out = bytearray()
    patterns = [
        0, 1, -1, 32767, -32768, 256, -256, 16384, -16384,
        1234, -4321, 321, 8191, -8192, 511, -512,
    ]
    for i in range(n):
        if i % 3 == 0:
            v = patterns[i % len(patterns)]
        else:
            v = ((i * 977) % 65536) - 32768
        out += struct.pack("<h", v)
    return bytes(out)


def seed(row, flags, frames):
    b = bytearray()
    b.append(row & 0xFF)
    b.append(flags & 0xFF)
    b += struct.pack(">H", (frames - 1) % 2048)
    b += sample_stream(96)
    return bytes(b)


def main():
    out_dir = sys.argv[1] if len(sys.argv) > 1 else "."
    zip_path = os.path.join(out_dir, "sndfile_rw_fuzzer_seed_corpus.zip")

    entries = []
    for row in range(ROW_COUNT):
        # Frugal layout: mono, no options, small frame count.
        entries.append(("rw_seed_%03d_frugal" % row, seed(row, 0x00, 48)))
        # Option-heavy layout: stereo, dither, PEAK/broadcast/update bits,
        # rate bit; the harness masks flags a row does not allow.
        entries.append(
            (
                "rw_seed_%03d_optioned" % row,
                seed(row, 0x2F | 0x80, 97),
            )
        )
    # A few wildcard seeds with extreme frame counts.
    for row in (0, 9, 20, 45, 54, 76):
        entries.append(("rw_seed_%03d_bigframes" % row, seed(row, 0x24, 2048)))

    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zf:
        for name, blob in entries:
            zf.writestr(name, blob)

    print("sndfile_rw_seeds: wrote %d seeds to %s" % (len(entries), zip_path))


if __name__ == "__main__":
    main()
