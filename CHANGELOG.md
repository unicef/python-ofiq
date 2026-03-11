# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.0] - 2026-03-11

### Added

- Initial release
- `OFIQ` class with one-time initialization
- `scalar_quality()` for unified quality score
- `vector_quality()` for all quality measures with optional raw scores
- Custom exception hierarchy (`OFIQError`, `ConfigError`, `QualityError`, `FaceDetectionError`, `LandmarkError`, `SegmentationError`)
- `python-ofiq setup` CLI command to download models and config
- `ofiq.setup()` and `ofiq.resolve_data_dir()` Python API
- Auto-discovery of data directory via `OFIQ_DATA_DIR` env var or `~/.ofiq/data`
- Type stubs for IDE support
- Prebuilt wheels for Linux x86_64 (manylinux_2_28), Python 3.11 and 3.12

### Compatibility

- Built against OFIQ C++ library version 1.1.2

[Unreleased]: https://github.com/unicef/python-ofiq/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/unicef/python-ofiq/releases/tag/v0.1.0
