---
layout: home
title: The libsndfile Home Page
---

Libsndfile is a C library for reading and writing files containing sampled sound
(such as MS Windows WAV and the Apple/SGI AIFF format) through one standard
library interface. It is released in source code format under the
[Gnu Lesser General Public License](http://www.gnu.org/copyleft/lesser.html).

The library was written to compile and run on a Linux system but should compile
and run on just about any Unix (including MacOS X).
There are also pre-compiled binaries available for 32 and 64 bit windows.

It was designed to handle both little-endian (such as WAV) and big-endian (such
as AIFF) data, and to compile and run correctly on little-endian (such as Intel
and DEC/Compaq Alpha) processor systems as well as big-endian processor systems
such as Motorola 68k, Power PC, MIPS and Sparc. Hopefully the design of the
library will also make it easy to extend for reading and writing new sound file
formats.

It has been compiled and tested (at one time or another) on the following
systems:

* Every platform supported by Debian GNU/Linux including x86_64-linux-gnu,
  i486-linux-gnu, powerpc-linux-gnu, sparc-linux-gnu, alpha-linux-gnu,
  mips-linux-gnu and armel-linux-gnu.
* powerpc-apple-darwin7.0 (Mac OS X 10.3)
* sparc-sun-solaris2.8 (using gcc)
* mips-sgi-irix5.3 (using gcc)
* QNX 6.0
* i386-unknown-openbsd2.9
* Microsoft Windows

At the moment, each new release is being tested on i386 Linux, x86_64 Linux,
PowerPC Linux, Win32 and Win64.

## Features

libsndfile has the following main features :

* Ability to read and write a large number of [file formats](formats.html).
* A simple, elegant and easy to use Applications Programming Interface.
* Usable on Unix, Win32, MacOS and others.
* On the fly format conversion, including endian-ness swapping, type conversion
  and bitwidth scaling.
* Optional normalisation when reading floating point data from files containing
  integer data.
* Ability to open files in read/write mode.
* The ability to write the file header without closing the file (only on files
  open for write or read/write).
* Ability to query the library about all supported formats and retrieve text
  strings describing each format.

libsndfile has a comprehensive test suite so that each release is as bug free
as possible.
When new bugs are found, new tests are added to the test suite to ensure that
these bugs don't creep back into the code.
When new features are added, tests are added to the test suite to make sure that
these features continue to work correctly even when they are old features.

## History

My first attempt at reading and writing WAV files was in 1990 or so under
Windows 3.1. I started using Linux in early 1995 and contributed some code to
the [wavplay](http://www.vaxxine.com/ve3wwg/gnuwave.html) program. That
contributed code would eventually mutate into this library. As one of my
interests is Digital Signal Processing (DSP) I decided that as well as reading
data from an audio file in the native format (typically 16 bit short integers)
it would also be useful to be able to have the library do the conversion to
floating point numbers for DSP applications. It then dawned on me that whatever
file format (anything from 8 bit unsigned chars, to 32 bit floating point
numbers) the library should be able to convert the data to whatever format the
library user wishes to use it in. For example, in a sound playback program, the
library caller typically wants the sound data in 16 bit short integers to dump
into a sound card even though the data in the file may be 32 bit floating point
numbers (ie Microsoft's WAVE_FORMAT_IEEE_FLOAT format). Another example would be
someone doing speech recognition research who has recorded some speech as a 16
bit WAV file but wants to process it as double precision floating point numbers.

Here is the release history for libsndfile:

* Version 0.0.8 (Feb 15 1999) First official release.
* Version 0.0.28 (Apr 26 2002) Final release of version 0 of libsndfile.
* Version 1.0.0rc1 (Jun 24 2002) Release candidate 1 of version 1 of libsndfile.
* Version 1.0.0rc6 (Aug 14 2002) MacOS 9 fixes.
* Version 1.0.0 (Aug 16 2002) First 1.0.X release.
* Version 1.0.1 (Sep 14 2002) Added MAT4 and MAT5 file formats.
* Version 1.0.2 (Nov 24 2002) Added VOX ADPCM format.
* Version 1.0.3 (Dec 09 2002) Fixes for Linux on ia64 CPUs.
* Version 1.0.4 (Feb 02 2003) New file formats and functionality.
* Version 1.0.5 (May 03 2003) One new file format and new functionality.
* Version 1.0.6 (Feb 08 2004) Large file fix for Linux/Solaris, new
  functionality   and Win32 improvements.
* Version 1.0.7 (Feb 24 2004) Fix build problems on MacOS X and fix ia64/MIPS
  etc clip mode detction.
* Version 1.0.8 (Mar 14 2004) Minor bug fixes.
* Version 1.0.9 (Mar 30 2004) Add AVR format. Improve handling of some WAV
  files.
* Version 1.0.10 (Jun 15 2004) Minor bug fixes. Fix support for Win32 MinGW
  compiler.
* Version 1.0.11 (Nov 15 2004) Add SD2 file support, reading of loop data in WAV
  and AIFF. Minor bug fixes.
* Version 1.0.12 (Sep 30 2005) Add FLAC and CAF file support, virtual I/O
  interface. Minor bug fixes and cleanups.
* Version 1.0.13 (Jan 21 2006) Add read/write of instrument chunks. Minor bug
  fixes.
* Version 1.0.14 (Feb 19 2006) Minor bug fixes. Start shipping windows
  binary/source ZIP.
* Version 1.0.15 (Mar 16 2006) Minor bug fixes.
* Version 1.0.16 (Apr 30 2006) Add support for RIFX. Other minor feature
  enhancements and bug fixes.
* Version 1.0.17 (Aug 31 2006) Add C++ wrapper sndfile.hh. Minor bug fixes and
  cleanups.
* Version 1.0.18 (Feb 07 2009) Add Ogg/Vorbis suppport, remove captive
  libraries, many new features and bug fixes. Generate Win32 and Win64
  pre-compiled binaries.
* Version 1.0.19 (Mar 02 2009) Fix for CVE-2009-0186. Huge number of minor fixes
  as a result of static analysis.
* Version 1.0.20 (May 14 2009) Fix for potential heap overflow.
* Version 1.0.21 (December 13 2009) Bunch of minor bug fixes.
* Version 1.0.22 (October 04 2010) Bunch of minor bug fixes.
* Version 1.0.23 (October 10 2010) Minor bug fixes.
* Version 1.0.24 (March 23 2011) Minor bug fixes.
* Version 1.0.25 (July 13 2011) Fix for Secunia Advisory SA45125. Minor bug
  fixes and improvements.
* Version 1.0.26 (November 22 2015) Fix for CVE-2014-9496, CVE-2014-9756 and
  CVE-2015-7805. Add ALAC/CAF support. Minor bug fixes and improvements.
* Version 1.0.27 (June 19 2016) Fix a seek regression in 1.0.26. Add metadata
  read/write for CAF and RF64. FIx PAF endian-ness issue.
* Version 1.0.28 (April 2 2017) Fix buffer overruns in FLAC and ID3 handling
  code. Reduce default header memory requirements. Fix detection of Large File
  Support for 32 bit systems.
* Version 1.0.29 (August 15 2020) Opus support, build system improvements and
  bug fixes.
* Version 1.0.30 (September 19 2020) Bugfix release. Fix file descriptor leaks
  in sf_open_fd () function. Fix critical CMake bug leading to broken ABI on
  Linux platforms. Other numerous fixes to CMake build system, consider it
  stable now. Fix some memory leaks. Fix handling of some SD2 files. Update
  documentation. Integrate GitHub Actions for faster test builds and Oss-Fuzz
  for fuzzing tests. Move sndfile.h.in from src/ to include/ directory. To avoid
  problems, delete old generated sndfile.h from $(top_builddir)/src.

## Similar or Related Projects

* [SoX](http://sox.sourceforge.net/) is a program for converting between sound
  file formats.
* [Wavplay](http://www.hitsquad.com/smm/programs/WavPlay/) started out as a
  minimal WAV file player under Linux and has mutated into Gnuwave, a
  client/server application for more general multimedia and games sound
  playback.
* [Audiofile](http://www.68k.org/~michael/audiofile/) (libaudiofile) is a
  library similar to libsndfile but with a different programming interface. The
  author Michael Pruett has set out to clone (and fix some bugs in) the
  libaudiofile library which ships with SGI's IRIX OS.
* [sndlib.tar.gz](ftp://ccrma-ftp.stanford.edu/pub/Lisp/sndlib.tar.gz) is
  another library written by Bill Schottstaedt of CCRMA.

## Licensing

libsndfile is released under the terms of the GNU Lesser General Public License,
of which there are two versions;
[version 2.1](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
and
[version 3](http://www.gnu.org/copyleft/lesser.html).
To maximise the compatibility of libsndfile, the user may choose to use
libsndfile under either of the above two licenses.
You can also read a simple explanation of the ideas behind the GPL and the LGPL
[here](http://www.gnu.org/copyleft/lesser.html).

You can use libsndfile with
[Free Software](http://www.gnu.org/),
[Open Source](http://www.opensource.org/),
proprietary, shareware or other closed source applications as long as libsndfile
is used as a dynamically loaded library and you abide by a small number of other
conditions (read the LGPL for more info).
With applications released under the GNU GPL you can also use libsndfile
statically linked to your application.

I would like to see libsndfile used as widely as possible but I would prefer it
if you released software that uses libsndfile as
[Free Software](http://www.gnu.org/)
or
[Open Source](http://www.opensource.org/).
However, if you put in a great deal of effort building a significant application
which simply uses libsndfile for file I/O, then I have no problem with you
releasing that as closed source and charging as much money as you want for it as
long as you abide by [the license](http://www.gnu.org/copyleft/lesser.html).

## Download

Check latest version on
[GitHub Releases page](https://github.com/libsndfile/libsndfile/releases/).

## See Also

* [sndfile-tools](https://github.com/libsndfile/sndfile-tools): a small
collection of programs which use libsndfile.
