#!/usr/bin/make -f

export AFL_HARDEN = 1
export AFL_USE_ASAN = 1
# export AFL_USE_MSAN = 0

export AFL_SKIP_CPUFREQ = 1

export CC = afl-gcc
export CXX = afl-g++

fuzz : programs/sndfile-info
	mkdir -p afl/out
	afl-fuzz -t 400 -m -1 -i afl/in -o afl/out -- programs/sndfile-info @@

configure : configure.ac
	./autogen.sh

config.status : configure
	Scripts/static-deps-build.mk config
	Scripts/static-deps-build.mk clean

programs/sndfile-info : config.status
	Scripts/static-deps-build.mk build
