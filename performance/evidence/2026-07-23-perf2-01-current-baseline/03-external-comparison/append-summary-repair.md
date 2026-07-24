# Common timing append summary repair

All 30 append measurements exited zero and passed their correctness gates. The
initial summary step failed because the three append-only manifest rows were
mistakenly marked `aggregate=yes`, so the summarizer correctly required absent
ooRexx reference cells. No measurement command, work value, runtime, image or
raw sample was defective.

The manifest flags were corrected to the governed append form
`aggregate=no`. `common-noise-append-summary/` was then regenerated from the
unchanged `common-noise-append/samples.csv`, and `common-combined/` was derived
from the initial and append sample files. No sample was rerun, removed or
changed. The failed capture-manifest result is retained as the exact record of
the first summary attempt; this note and the successful regenerated summaries
are its disposition.
