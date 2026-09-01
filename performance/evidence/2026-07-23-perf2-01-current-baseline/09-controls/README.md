# RexxCPS family controls

`rexxcps_family_controls.crexx` is an exact-hash cREXX Level B attribution
control. It covers BIFs, internal calls/argument parsing, TRACE/ADDRESS, stems,
decimal/string loops and PARSE. Each family uses a nominal
`RexxCPS-2.2d-1000-clauses` accounting label and declares
`publication=attribution-only`.

Exact hashes are source
`878ad5dc826e9ecc1fd57d0bdaf9662b0ab65256f95149a589ad02ed40584d0c`,
RXAS `07ba5b4493e7df9b1e3e48437485c72ef0092835cff95a3210066633d0c17b13`
and RXBIN
`a9e29113f80b0542f387ed99c8b89a3ec1a6d36424a5d9ed25a1533b2add85e7`.
Both VMs passed correctness and schema-5 counts capture for every family. The
ordinary product timing lane used two warmups, five recorded runs and the governed
unchanged append for flagged cells. Remaining noise labels are retained.

These probes isolate coverage and provide candidate guards; they do not remove
one family from the full RexxCPS program, are not additive/subtractive ceilings,
and never replace the published cREXX RexxCPS 2.2d result.
