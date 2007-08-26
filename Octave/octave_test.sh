#!/bin/bash


# Check where we're being run from.
if [ -d Octave ]; then
	cd Octave
	fi

if [ ! -f sfread.cc ]; then
	echo "Can't find sfread.cc. Probably being run from incorrect dir."
	exit 1
	fi

# Find libsndfile shared object.
libsndfile_so_location=""

if [ -f "../src/.libs/libsndfile.so" ]; then
	libsndfile_so_location="../src/.libs/"
elif [ -f "../src/libsndfile.so" ]; then
	libsndfile_so_location="../src/"
else
	echo "Not able to find the libsndfile.so we've just built."
	exit 1
	fi
libsndfile_so_location=`(cd $libsndfile_so_location && pwd)`


# Find sfread.oct and sfwrite.oct
sfread_write_oct_location=""

if [ -f .libs/sfread.oct ]; then
	sfread_write_oct_location=".libs"
elif [ -f sfread.oct ]; then
	sfread_write_oct_location="."
else
	echo "Not able to find the sfread.oct/sfwrite.oct binaries we've just built."
	exit 1
	fi

case `file -b $sfread_write_oct_location/sfread.oct` in
	ELF*)
		;;
	*)
		echo "Not able to find the sfread.oct/sfwrite.oct binaries we've just built."
		exit 1
		;;
	esac

# echo "libsndfile_so_location : $libsndfile_so_location"
# echo "sfread_write_oct_location : $sfread_write_oct_location"

LD_LIBRARY_PATH="$libsndfile_so_location:$LD_LIBRARY_PATH"

octave_script="`pwd`/octave_test.m"

(cd $sfread_write_oct_location && octave -qH $octave_script)


