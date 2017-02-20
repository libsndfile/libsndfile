# libsndfile

[![Build Status](https://secure.travis-ci.org/erikd/libsndfile.svg?branch=master)](http://travis-ci.org/erikd/libsndfile)

libsndfile is a C library for reading and writing files containing sampled audio
data.

## Hacking

The canonical source code repository for libsndfile is at
[https://github.com/erikd/libsndfile/][github].

You can grab the source code using:

    $ git clone git://github.com/erikd/libsndfile.git

For building for Android see [BuildingForAndroid][BuildingForAndroid].

There are currently two build systems; the official GNU autotool based one and
a more limited and experimental CMake based build system. Use of the CMake build
system is documented below.

Setting up a build environment for libsndfile on Debian or Ubuntu is as simple as:
```
sudo apt install autoconf autogen automake build-essential libasound2-dev \
    libflac-dev libogg-dev libtool libvorbis-dev pkg-config python
````
For other Linux distributions or any of the *BSDs, the setup should be similar
although the package install tools and package names may be slightly different.

Similarly on Mac OS X, assuming [brew] is already installed:
```
brew install autoconf autogen automake flac libogg libtool libvorbis pkg-config
```
Once the build environment has been set up, building and testing libsndfile is
as simple as:

    $ ./autogen.sh
    $ ./configure --enable-werror
    $ make
    $ make check


## The CMake build system.

The CMake build system is still experimental and probably only works on linux
because it still relies on GNU autotools for bootstrapping. Using it as simple
as:

    $ Scripts/cmake-build.sh

I would be happy to accept patches to make the CMake build system more portable.


## Submitting Patches.

* Patches should pass all pre-commit hook tests.
* Patches should always be submitted via a either Github "pull request" or a
  via emailed patches created using "git format-patch".
* Patches for new features should include tests and documentation.
* Patches to fix bugs should either pass all tests, or modify the tests in some
  sane way.
* When a new feature is added for a particular file format and that feature
  makes sense for other formats, then it should also be implemented for one
  or two of the other formats.





[brew]: http://brew.sh/
[github]: https://github.com/erikd/libsndfile/
[BuildingForAndroid]: https://github.com/erikd/libsndfile/blob/master/Building-for-Android.md
