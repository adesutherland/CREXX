# Peter Codex Notes

This is a personal notebook for future maintenance ideas, repo observations, and follow-up thoughts.

It complements `AGENTS.md` but never replaces it.

`AGENTS.md` remains the canonical repository instruction file. When something here looks useful for the official workflow, record it here first and let the project architect decide whether to adopt it.

If there are cREXX-specific differences or workflow notes that apply,
briefly list them before proposing an implementation.

This notebook is intended to be consulted selectively.


Each task should use only the sections relevant to its scope.git fetch

## Working Philosophy

- Stay practical.
- Prefer repository truth over generic model memory.
- Keep notes observational, not authoritative.
- Treat the notebook as a memory aid for future me, not as project policy.
- When a task touches multiple subsystems, map the boundary before editing.

## Standard Task Workflow

- Read the smallest relevant docs first.
- Inspect nearby code and tests before editing.
- Preserve local style.
- Make the smallest safe change.
- Verify with the focused test or build path that matches the subsystem.
- Record anything surprising so it is not lost.

## Task Checklists

See also:

- `docs/ai-context/CREXX_ARCHITECTURE.md`
- `docs/ai-context/CREXX_LEVELB_AUTHORING.md`
- `compiler/docs/testing.md`
- `docs/ai-context/CREXX_LIBS.md`

### Compiler Change

- Find the smallest repro.
- Check the relevant compiler docs and nearby tests.
- Identify whether the change touches parsing, validation, inlining, or emission.
- Run the narrowest compiler regression that covers the path.
- Note any behavior shift that feels like a semantics change.

### Assembler Change

- Confirm the input shape and expected RXAS output.
- Check for metadata, operand, or optimization effects.
- Compare opt and noopt behavior when relevant.
- Verify the emitted bytecode or disassembly if the change affects encoding.

### VM or Runtime Change

- Identify whether the change touches value state, dispatch, metadata, signals, or loading.
- Check the focused runtime tests first.
- Watch for semantics changes that may also need library or compiler follow-up.
- Verify any behavior that crosses a module boundary.

### Library or BIF Change

- Preserve existing source comments and doc tags.
- Check the owning library docs and nearby examples.
- Verify the library test path that exercises the change.
- Confirm whether the change affects compiler lowering or runtime dispatch.

### RXPA or RXPP Change

- Check the subsystem-specific docs before editing.
- Keep the extension boundary clear.
- Verify the direct extension workflow, not just end-to-end behavior.
- Record any new conventions that future sessions should reuse.

## Prompt Library

- "Summarize the subsystem boundaries before changing code."
- "Identify the smallest reproducible case."
- "What docs already define this behavior?"
- "Which tests cover the changed path?"
- "What would break if we changed this assumption?"
- "Give me the narrowest safe plan for this change."
- "List the likely blast radius before editing."
- "Point out anything that belongs in AGENTS.md, but keep it in the notebook."
- "Show me the verification path I should run next."

## Lessons Learned

- `crexx` is organized as a multi-stage toolchain rather than a single compiler tree.
- The current architecture docs are the best source for compiler, VM, linker, RXPA, and RXPP behavior.
- Repo guidance is more useful when it names subsystem boundaries explicitly.
- Build and test expectations are clearer when they mention the subsystem-specific commands, not only the general CTest habit.
- The notebook is most useful when it stays workflow-oriented and light on policy.
- Small prompts and checklists are easier to reuse than long narrative notes.

## Things Codex Did Well

- Kept the notebook separate from `AGENTS.md`.
- Captured the repo structure and subsystem concerns as observations instead of rules.
- Preserved the AGENTS.md improvement ideas with dates and wording.

## Things Codex Did Wrong

- Over-focusing on `AGENTS.md` proposals instead of making the notebook more useful day to day.
- Not organizing the notes into workflow-oriented sections earlier.
- Leaning too hard on the official-instructions framing for a personal notebook.

## Possible improvements to AGENTS.md

### 2026-07-26

- Why I think it would help: The current instructions mention the four main binaries, but not the full subsystem layout. A clearer map would help avoid accidental cross-layer edits.
- Proposed wording:
  - "Repository boundaries: `compiler/` owns parsing, validation, exits, inlining, and emission; `assembler/` owns `.rxas` to `.rxbin`; `linker/` owns image selection and module linking; `interpreter/` owns `rxvm`; `preprocessor/` owns `rxpp`; `rxpa/` owns native extension APIs; `lib/` owns BIFs and runtime libraries; `rexxscript/` is a separate product; `tests/` and `docs/` contain verification and documentation."

### 2026-07-26

- Why I think it would help: Platform behavior has become explicit in the docs, especially Windows build/test overlap, ELF runpaths, install-prefix defaults, and platform-specific TLS choices.
- Proposed wording:
  - "Supported platforms are Windows, macOS, and Linux. Preserve platform-specific install, runpath, and TLS-backend behavior; on Windows, avoid overlapping build/test runs in the same build tree; on ELF systems, preserve the `$ORIGIN` runpath used by installed helpers."

### 2026-07-26

- Why I think it would help: The repo now has concrete verification expectations by subsystem, and the current guidance is still too general.
- Proposed wording:
  - "Before considering a task complete, run the focused verification that matches the subsystem: compiler regressions for `compiler/` changes, assembler tests for `assembler/`, linked-image tests for `linker/`, runtime and VM tests for `interpreter/`, library/BIF tests for `lib/`, and the documented RXPP/ASAN workflows when those areas change."

### 2026-07-26

- Why I think it would help: The codebase now treats several language contracts as locked behavior, and the instructions should make that explicit for future edits.
- Proposed wording:
  - "Do not change Rexx semantics casually. Preserve the current Level B contracts for typed `.string` versus `.binary`, source-map handling, `DO ... END` expression blocks, interface/factory dispatch, ADDRESS environment behavior, and the separation between Level B, Level C, and Level G responsibilities."

### 2026-07-26

- Why I think it would help: Generated and vendored code are mixed into the tree, so a no-hand-edit rule would prevent accidental churn.
- Proposed wording:
  - "Do not manually edit generated, copied, or vendored artifacts unless the task explicitly requires regenerating them. This includes parser/lexer outputs, build-tree generated files, message catalogs, bundled third-party code, and other derived artifacts."

### 2026-07-26

- Why I think it would help: BIF and plugin work crosses several implementation layers, and a short reminder would reduce bootstrap mistakes.
- Proposed wording:
  - "For BIF and plugin changes, preserve RexxDoc comments and keep the split between RXPA native extensions, compiler exits, preprocessor behavior, and library-owned Level B code clear. When adding or modifying BIFs, update the owning library docs and the relevant tests together."

## Follow-Up Ideas

- If this notebook proves useful, add a short checklist for common task types:
  - compiler logic
  - assembler changes
  - linker changes
  - VM/runtime changes
  - RXPA/plugin work
  - RXPP/preprocessor work
  - library/BIF changes
- Keep the notes observational, not authoritative.
- Treat AGENTS.md as the only canonical instruction source.

## cREXX Differences from Classic Rexx

### Arrays and stems

- Do not manually set `array[0]` after changing the number of elements.
- For stems, use `size()` or the key/value APIs; the current stem contract does not define a writable `stem.0` count slot.
- In cREXX, collection shape and count are maintained by the implementation rather than by manual count-slot updates.

References:

- `lib/rxfnsb/rexx/stem.md`
- `docs/ai-context/CREXX_LEVELB_AUTHORING.md`
- `lib/rxfnsb/rexx/objectarraydrop.md`

## Session Notes

## Decisions

## Open Threads

## Templates

## Open Questions

## Scratchpad

## Follow-Up Queue
