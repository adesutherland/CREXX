# Maintained research inputs

These are live regression inputs, not saved runs. `config-*` and `consumers-*`
are independent recognition/value oracles; `recovery-*`, `lexrecovery-*` and
`lrrepair-*` specify deterministic damage/search corpora. `script-*` records
RexxScript input/expected observations. `pika-core-*`, `c14-*` and
`squirrel-expected.txt` provide algorithm comparison oracles. `migration/` and
`rexxfront/expected.txt` are closed-corpus projections used by the AST migration
consumers. Their scope is narrower than full compiler equivalence.

The adjacent Python generators compare expected content by default; `--update`
requires reviewing a changed oracle. Native compiler controls never override
an independently specified language contract automatically. Raw traces and
intermediate JSON are generated temporarily and deleted.

The pinned `mike-sources.json` identifies external/reference source revisions.
RexxScript controls use explicitly selected current runtime inputs, never an
archived binary snapshot. Their output must be compared with the maintained
oracle before making a new equivalence claim.
