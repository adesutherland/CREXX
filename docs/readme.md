# CREXX Documentation Sources

This directory is the root of the public GitHub Pages documentation site.

For current release documentation, start with [docs/index.md](index.md). For
the intended purpose of each documentation area, see
[DOCS_MAP.md](DOCS_MAP.md).

## Maintained Areas

- `books/crexx_language_reference`: implemented Level B language reference
- `books/crexx_programming_guide`: build, run, tools, and integration guide
- `books/crexx_vm_spec`: VM, RXAS, RXBIN, and instruction reference
- `ai-context`: implementation facts for agents and maintainers

## Generated and Historical Material

Some files under `docs/` and `docs/books/` are generated output, old source
material, or third-party reference material. Those files are retained when they
are useful to maintainers, but they are not automatically authoritative for the
current release.

When public docs disagree with code, tests, or `docs/ai-context`, verify the
implementation and update the maintained release docs.
