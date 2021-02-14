# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

* This `CHANGELOG.md`. All notable changes to this project will be documented in
  this file. The old `NEWS` file has been renamed to `NEWS.OLD` and is no longer
  updated.

### Changed

* `SFC_SET_DITHER_ON_READ` and `SFC_SET_DITHER_ON_WRITE` enums comments in
  public header (#677).

### Fixed

* Typo in `docs/index.md`.
* Memory leak in `caf_read_header`(), credit to OSS-Fuzz ([issue 30375](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=30375)).
* Normalisation issue when scaling floating point data to `int` in
  `replace_read_f2i`() (#702).

[Unreleased]: https://github.com/libsndfile/libsndfile/compare/1.0.31...HEAD
