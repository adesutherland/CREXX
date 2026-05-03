# CREXX Documentation Map

This map defines the purpose of each documentation area. It is intended to keep
release documentation, implementation notes, agent context, and project history
from drifting into one another.

## Trust Order

For current implementation facts, trust sources in this order:

1. Code and tests.
2. `docs/ai-context`.
3. Current release docs under `docs/books`.
4. `compiler/docs` when the question is compiler-internal.
5. The wiki only for vision, direction, and history.

If two documents disagree about implemented behaviour, verify against tests or
code before editing user-facing docs.

## Areas

| Area | Purpose | Audience | Contents | Tone |
| --- | --- | --- | --- | --- |
| `README.md` | GitHub front door | New visitors, contributors | What CREXX is, current baseline, build/run, links | Short and welcoming |
| `docs/index.md` | GitHub Pages landing page | Users and contributors | Release documentation index | Clear and navigable |
| `docs/releases` | Release notes | Users, packagers, contributors | Milestone summaries, release scope, signing status, known beta limitations | Concise and release-focused |
| `docs/books/crexx_language_reference` | Language reference | CREXX programmers | Syntax, types, statements, classes, libraries | Formal, as-implemented |
| `docs/books/crexx_programming_guide` | Practical guide | Users and integrators | Build, run, tools, host integration, plugins | Practical and task-oriented |
| `docs/books/crexx_vm_spec` | VM and bytecode reference | Implementers | VM model, instruction set, platform notes | Precise and technical |
| `docs/ai-context` | Current implementation context | Agents and maintainers | Architecture facts, debugging, Level B authoring, library/runtime notes | Operational and explicit |
| `compiler/docs` | Compiler implementation notebook | Compiler maintainers | Parser, validation, emitter, exits, inlining, retired working notes | Deep technical notes with status labels |
| `docs/bifs`, `docs/functions`, `docs/instructions` | Data and generation assets | Maintainers | Reference inputs and old generation data | Source material, not prose docs |
| GitHub wiki | Vision, direction, and history | Project followers | Why CREXX exists, where it is going, historical milestones | Contextual, not current technical reference |
| `AGENTS.md` | Repository working rules | Agents and contributors | Required trust sources, workflow rules, debug discipline | Canonical and concise |

## Promotion Rules

- User-facing facts belong in `docs/books` and are linked from
  `docs/index.md`.
- Release milestone summaries belong in `docs/releases` and may be copied into
  GitHub Releases.
- Implementation facts needed by agents belong in `docs/ai-context`.
- Deep compiler mechanisms belong in `compiler/docs`; promote summaries into
  public docs only when they help users or integrators.
- Working notes may stay in `compiler/docs` or `docs/ai-context`, but they must
  carry a status line if they are not final.
- The wiki must not contain current command syntax, grammar, VM internals,
  build recipes, or API facts. Link to release docs instead.

## Current Cleanup Status

The first Release 1 beta documentation pass updated the public front doors and
pruned the wiki to project vision/history. The second pass promotes selected
as-built technical content from `docs/ai-context` into the language reference,
programming guide, and VM specification. Further work should focus on detailed
examples and optional plugin pages.
