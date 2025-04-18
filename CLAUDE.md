# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

- Build: `mkdir build; meson setup build && meson compile -C build`
- Rebuild: `rm -fr build; mkdir build; meson setup build && meson compile -C build`
- Run: `build/dbs26 [options]`
- Help: `build/dbs26 --help`
- Benchmark (no disk output): `build/dbs26 -b`

## Code Style Guidelines

- License header: // SPDX-License-Identifier (LGPL-3.0-or-later)
- Prefer C23 when supported, otherwise prefer C23 compat macros
- Use only tabs for indentation and only spaces for alignment
  - Never mix tabs and spaces
  - Never assume a specific tab width
- When in doubt follow the existing coding style
- Use compatibility and helper macros found in `src/compat.h`, `src/util.h`, etc.
- Write cross-platform compatible code using the provided compat macros
- Header guard macro naming convention is `<REPO>_<SUBDIR>_<MODULE>_H_`
  - Example: `src/foobar.h` has guard macro `DBS26_SRC_FOOBAR_H_`
- Write thorough Doxygen documentation with `@brief`, `@param`, `@return`, etc.
- Use modular snake_case function naming convention
  - Module name is function name prefix: foo getter in `src/bar.h` is `bar_get_foo()`
