# Use `just <recipe>` to run a recipe
# https://just.systems/man/en/

import ".shared/common.just"
import ".shared/cpp.just"

# By default, run the `--list` command
default:
    @just --list

# Fix formatting of the source CMake file
[group('linter')]
cmake-format-fix:
    cmake-format -i CMakeLists.txt
