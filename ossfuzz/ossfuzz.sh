#!/usr/bin/env sh

set -eu

# This script is called by the oss-fuzz main project when compiling the fuzz
# targets. This script is regression tested by ci_oss.sh.

# Save off the current folder as the build root.
export BUILD_ROOT="${PWD}"

echo "CC: ${CC:-}"
echo "CXX: ${CXX:-}"
echo "LIB_FUZZING_ENGINE: ${LIB_FUZZING_ENGINE:-}"
echo "CFLAGS: ${CFLAGS:-}"
echo "CXXFLAGS: ${CXXFLAGS:-}"
echo "OUT: ${OUT:-}"

MAKEFLAGS="${MAKEFLAGS:+${MAKEFLAGS} }-j$(nproc)"
export MAKEFLAGS

# Install dependencies
apt-get -y install autoconf autogen automake libtool pkg-config python

# For now, do not install the following libraries (as they won't be in the
# final image):
# libasound2-dev libflac-dev libogg-dev libopus-dev libvorbis-dev

# Compile the fuzzer.
autoreconf -vif
./configure --disable-shared --enable-ossfuzzers
make V=1

# Copy the fuzzer to the output directory.
cp -v ossfuzz/sndfile_fuzzer "${OUT}/"
cp -v ossfuzz/sndfile_alt_fuzzer "${OUT}/"
cp -v ossfuzz/sndfile_fmt_fuzzer "${OUT}/"
cp -v ossfuzz/sndfile_cmd_fuzzer "${OUT}/"
cp -v ossfuzz/sndfile_cmd_fuzzer.dict "${OUT}/"

# Seed corpus for the command-surface harness: minimal valid containers with
# control words for each command group (see sndfile_cmd_seeds.py). Optional:
# a failure to build the corpus must not fail the fuzzers themselves.
if command -v python3 >/dev/null 2>&1 ; then
	python3 ossfuzz/sndfile_cmd_seeds.py "${OUT}" || echo "seed generation failed (non-fatal)"
fi
