# Repository Guidelines

## Project Structure & Module Organization
PRQLite is a C++17 relational database engine. Public headers live in `include/`, grouped by subsystem such as `frontend/`, `backend/`, `virtual_machine/`, `transaction/`, and `recovery/`. Implementations live in matching paths under `src/`; the executable target is assembled from `src/CMakeLists.txt`. Parser-focused tests are in `tests/test_*.cpp`. Persistent sample data is under `data/`. The `bloom_filter/` directory is a submodule built as a separate CMake target and linked into `db`.

## Build, Test, and Development Commands
- `cmake -S . -B build` configures the project and generates build files.
- `cmake --build build` builds the `db` executable and linked BloomFilter target.
- `.\run_tests.ps1` builds and runs all `tests/test_*.cpp` parser tests with `g++` or MSVC `cl`.
- `.\run_tests.ps1 -NoBuild` reruns previously built test executables from `build/tests`.
- `docker build -t prqlite:latest .` builds the container image described in `Dockerfile`.
- `docker run -it -v prqlite-data:/data prqlite:latest` starts PRQLite with persistent storage.

- For normal testing after implementing any feature don't follow above methods unless asked simply do `cmake --build ./build --config Debug` 

## Coding Style & Naming Conventions
Use modern C++17 and keep headers in `include/` paired with implementations in `src/`. Follow the existing lowercase snake_case file naming style, for example `buffer_pool.cpp`, `semantic_analyzer.hpp`, and `create_index.cpp`. Prefer subsystem directories over flat additions. CMake enables strict warnings: `/W4 /permissive-` on MSVC and `-Wall -Wextra -Wpedantic` elsewhere, so keep new code warning-clean. Match the local indentation and brace style in neighboring files.
Don't make unncessary .MD or documentation files in the repo unless asked for.
NEVER WRITE CODE IMPLEMENTATIONS IN .HPP FILES!

## Testing Guidelines
Tests are standalone C++ files named `test_<feature>.cpp` under `tests/`. Add focused tests near the subsystem behavior being changed, especially parser, lexer, semantic analysis, and execution paths. The PowerShell runner treats output containing `[FAIL]` as a failure, so keep test result markers consistent with existing tests.
If doing isolation/data integrity tests make sure add all changes so you can git statsh in case of data corruptions.

## Commit & Pull Request Guidelines
Recent commits use concise conventional-style prefixes such as `feat:` followed by an imperative summary, for example `feat: implement DELETE operator`. Keep commits scoped to one logical change. Pull requests should describe the behavioral change, list test commands run, mention affected subsystems, and include screenshots or terminal output only when they clarify user-visible behavior. Don't Add yourself as contributor or co-author in git commits.

## Security & Configuration Tips
Do not commit generated build artifacts, local database files beyond intentional fixtures, or secrets. Keep persistent runtime data in Docker volumes or `data/` fixtures, and avoid hard-coding machine-specific paths.
