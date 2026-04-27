# CREXX

_README File - 15 May 2021_

cREXX Version: cREXX-Phase-0 v0.1.4

## REXX Language Implementation Architecture

Project to develop a modern ground up implementation of a REXX
interpreter and compiler, and experiments with language improvements.
To implement REXX using the best language tools available
today, including the LLVM Compiler Infrastructure. These tools will
allow a REXX compiler to be produced supporting multiple backends
(including 64 bit architectures).

One aspect of the project is to revisit the REXX language - what can be
improved? And most importantly how can it be improved while keeping the
essence of REXX:

> to make programming easier than before

CREXX will be targeted to run on VM/370 (a nod to REXX's heritage)
and it will also run on Linux, Windows, OSX, and z/Architecture.

Please see our [project aims](../../wiki/Project-Aims).

## Wiki Based Documentation

This represents the latest thoughts, aims, architecture, designs and details; some of this may have been built, some things may have been built to older designs and some may just be a future wish. This is the place where we are developing the cREXX Architecture.

Key Links:

- [Wiki Home](https://github.com/adesutherland/CREXX/wiki "CREXX Wiki Home")
- [Issues](https://github.com/adesutherland/CREXX/issues "CREXX Issues")
- [Discussions](https://github.com/adesutherland/CREXX/discussions "CREXX Discussions")
- [Project Kanban](https://github.com/adesutherland/CREXX/projects/1 "CREXX Kanban")
- And of course the [Github Home](https://github.com/adesutherland/CREXX "CREXX Github Home")

## Release Based Documentation

This is the "As Built" documentation, specific to its release; the current [develop branch](https://adesutherland.github.io/CREXX/) version is available as a website.

The documentation is stored in the code repository/branch under the [/doc](https://github.com/adesutherland/CREXX/tree/develop/docs) directory as markdown files.

## DSL Syntax Highlighter Dependency

Parser mode uses the DSLSH middleware libraries from
`DSL-Syntax-Highlighter/codebuffer` and the `parser_tester` middleware tool. It
does not build DSLSH parser adapters.

If `../DSL-Syntax-Highlighter` exists, CMake uses that local checkout. If not,
CMake fetches DSLSH from `DSLSH_GIT_REPOSITORY` at `DSLSH_GIT_TAG`.

```bash
# Follow the configured branch, currently develop.
cmake -S . -B cmake-build-debug

# Pin to a stable tag or exact WIP commit.
cmake -S . -B cmake-build-debug -DDSLSH_GIT_TAG=<tag-or-commit-sha>

# Ignore a sibling checkout and fetch the configured ref.
cmake -S . -B cmake-build-debug -DDSLSH_PREFER_LOCAL=OFF
```

Use a branch name for easy "latest DSLSH" development. Use a tag or commit SHA
when CREXX needs a reproducible DSLSH version.
