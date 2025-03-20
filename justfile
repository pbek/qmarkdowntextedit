# Use `just <recipe>` to run a recipe
# https://just.systems/man/en/

# By default, run the `--list` command
default:
    @just --list

# Aliases

alias fmt := format

# Format all files
[group('linter')]
format:
    nix-shell -p treefmt libclang nodePackages.prettier shfmt nixfmt-rfc-style taplo --run treefmt
