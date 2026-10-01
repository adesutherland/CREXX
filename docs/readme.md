# CREXX Documentation Sources

This directory is the root of the public GitHub Pages documentation site.

For current release documentation, start with [docs/index.md](index.md). For
the intended purpose of each documentation area, see
[DOCS_MAP.md](DOCS_MAP.md).

## Maintained Areas

- `books/crexx_language_reference`: implemented Level B language reference
- `books/crexx_programming_guide`: build, run, tools, and integration guide
- `books/crexx_vm_spec`: VM, RXAS, RXBIN, and instruction reference
- `reference/rxas`: human-authored RXAS reference source skeleton
- `ai-context`: implementation facts for agents and maintainers

## Source and generated assets

Keep the source and generation assets needed by the supported books and
references. `ai-context` is the canonical implementation-guide source, and the
book-local guide symlinks are required inputs to the current printed books.
Preserve these links; they do not duplicate the source files. Completed
investigations and superseded runs are available in Git history.

See [BUILDING-DOCS.md](BUILDING-DOCS.md) for source/generated distinctions,
the actual generation routes and executable examples.

When public docs disagree with code, tests or `docs/ai-context`, verify the
implementation and update the maintained release docs.
