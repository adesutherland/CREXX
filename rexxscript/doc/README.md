# RexxScript Documentation

Status: product documentation for the current Release 1 beta 3 WIP line.

This directory is the master documentation view for the RexxScript product.
General CREXX documentation should link here for RexxScript details rather
than duplicating the full command, runtime, and implementation descriptions.

## Documents

- [User guide](user-guide.md): running RexxScript from the command line,
  embedding it with the `REXXSCRIPT` command, direct runtime API usage,
  supported language features, and current limits.
- [Developer guide](developer-guide.md): source layout, runtime architecture,
  build and test workflow, BIF dispatch, sandbox variable-pool behavior, and
  extension rules.

## Documentation Rules

- Keep user-facing behavior in `user-guide.md`.
- Keep implementation and contributor guidance in `developer-guide.md`.
- Keep public RexxScript classes, procedures, and future BIF helpers documented
  with Javadoc-style comments in source where practical. The documentation
  generation path is expected to use tags such as `@param`, `@return`, and
  related API metadata.
- Keep `docs/ai-context/REXXSCRIPT*.md` as maintainer/agent context that points
  back to this directory for the product view.
