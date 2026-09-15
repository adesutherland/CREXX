# Queue record interpretation

`completed-queue-snapshot.json` is an earlier in-progress snapshot.
`remaining-queue-final.json` retains all fourteen terminal entries, including
the installed generation deadline failure; it is not a claim that every entry
passed. That failure is repaired and the complete package coverage is reconciled
in `../package-generation-continuation/combined-coverage.json`.

`remaining-queue-before-fixture-fix.json` retains the original normal old-host
fixture failure, diagnosed and repaired as S6-QA01. The first passing old-host
invocation shared a runner directory with the following embedding control;
`debug-rxllama_qualify_old_host/retention-note.md` identifies its separate retained
passing rerun. Preserve these distinctions when reusing the evidence.
