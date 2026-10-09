# Overnight QA repair evidence - 9 October 2026

The failed runs checked out develop `0aafdb155b07d2a345193a870cc47922e1e837c0`,
although the scheduled event belongs to master. `inventory.json` records the
job families, causes and passed/unrun boundary. `failed/` retains the original
LSan report, failed test names and bounded Deep diagnostic excerpts.

The Level C closeout baseline is local commit
`b1b8bed2dabf333850441480b1021e556d096715`. SAN-010 owns the leaking frame-control
unit. Its repair uses the ordinary compiler Context lifecycle, preserving the
frame emission/source assertions. No product language, host stdout or Unicode
conversion policy changed in this repair batch.

Windows controls in `windows-control.json` reproduce the UTF-8 assembly versus
ACP injection failure and preserve literal NUL/UTF-8/CR bytes while constructing
native CRT newline expectations. Controls on macOS are evidence for the cause;
actual native Windows completion remains a separate hosted gate.

The authoritative scope, acceptance and qualification status are in
`docs/planning/release-1/levelc-compatibility-worklist.md` under the overnight
repair phase. Full local QA has passed; `qualification.json` records the normal and supported
Linux sanitizer results, qualified inputs, decimal requalification and final
book hashes. Publication and native Windows/automatic CI proof remain in progress.
