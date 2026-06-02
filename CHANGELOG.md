# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.3.1] - 2026-06-02

### Fixed
- `infinite_counter` and `processor` used `TIME{0}` to construct zero time, which
  fails for types with a private single-argument constructor (e.g. `decimal<N>`).
  Changed to `TIME{}` (default constructor), which initialises to zero for all
  cdboost-compatible time types.

### Changed
- `cdboost::log::to_sim_double` replaced by `to_sim_string`: simulation time is now
  logged in the type's native string representation (`"1/10"` for rationals,
  `"0.100"` for decimal, full-precision decimal for float/double). The `sim_time`
  JSON field changes from a number to a quoted string. No per-type specialisation
  is required; any time type with `operator<<` works automatically.

## [0.3.0] - 2026-05-02

C++23 modernization — renamed namespace, removed Boost and embedded port,
structured NDJSON logging added.

### Added
- NDJSON structured logging via spdlog; `cdboost::log` wrapper emits machine-readable JSON lines
- `docs/log-format.md` documenting the NDJSON log schema
- GitHub Actions CI workflow (GCC 14, vcpkg, C++23) with output validation step
- vcpkg manifest for dependency management
- Catch2 unit test suite

### Changed
- Namespace renamed from `boost::simulation` to `cdboost`
- Build migrated to CMake C++23 with vcpkg; GCC 14 required
- Applied C++ Core Guidelines audit (sections F, I, C, ES)
- Applied clang-format to all sources
- Migrated tests from Boost.Test to Catch2
- `append_range` replaced with `ranges::copy + back_inserter` for GCC 13 compatibility
- `assert` in release code replaced with throwing exceptions per Core Guidelines

### Removed
- Boost dependency
- Embedded/mbed port
- Pinned vcpkg CI commit replaced with direct clone

## [0.2.0] - 2015-02-10

Initial PDEVS implementation (boost::simulation namespace, BJam build).

[Unreleased]: https://github.com/sdavtaker/cdboost/compare/v0.3.1...HEAD
[0.3.1]: https://github.com/sdavtaker/cdboost/compare/v0.3.0...v0.3.1
[0.3.0]: https://github.com/sdavtaker/cdboost/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/sdavtaker/cdboost/releases/tag/v0.2.0
