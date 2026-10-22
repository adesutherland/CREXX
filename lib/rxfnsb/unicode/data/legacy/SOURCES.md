# Legacy single-byte encoding sources

These mapping files are build inputs for the production `rxunicode` whole-value
codecs. They were imported from the TUTOR repository at commit
`c7e7aa451782d01a5b321c0d4ac77021ab11c8bf`, retaining TUTOR's Format A files
for Windows-1252, IBM437, and IBM850 and its IBM1047 source mapping.

- TUTOR repository: <https://github.com/JosepMariaBlasco/TUTOR>
- TUTOR encoding documentation:
  <https://github.com/JosepMariaBlasco/TUTOR/blob/c7e7aa451782d01a5b321c0d4ac77021ab11c8bf/doc/encodings/readme.md>
- TUTOR mapping directory:
  <https://github.com/JosepMariaBlasco/TUTOR/tree/c7e7aa451782d01a5b321c0d4ac77021ab11c8bf/bin/encodings/build>
- Checksums: [`SHA256SUMS`](SHA256SUMS)

The cREXX compiler accepts the TUTOR Format A source layout but emits its own
versioned immutable runtime image. Windows-1252's five source entries without a
Unicode target decode to the matching C1 controls, following TUTOR's retained
mapping policy. The three source files that arrived without a final newline
were newline-terminated locally; their cREXX checksums therefore intentionally
differ from the corresponding raw GitHub blobs.

Updating these inputs requires updating their checksums, prepared-image size
and mapping audits, all-byte round-trip tests, aliases, and user documentation
in one change.
