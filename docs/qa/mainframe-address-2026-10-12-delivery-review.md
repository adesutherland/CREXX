# Native fault-control delivery amendment

12 October 2026. The final 47-record tool pack and 25-record supplement remain
unchanged. This additional reviewed delta replaces only the native TSO
MFFLT31 containment-control package and its producer; guest execution remains
pending. It does not change cREXX runtime code or any of the eight tools.

Native restore of the earlier fault control did not list MFFLT31. The operator
did not CALL it. Its producer had passed an absolute output path to classic-ld,
the same member-name construction error repaired in the main tool producer.
The maintained SDK `libc/scripts/build-tso-fault-leaf.crexx` now supplies a
basename from the output directory and requires an explicit independent XMIT
reader. The reader checks member MFFLT31, AMODE 31 and RMODE ANY. Input/tool
hashes are checked before and after production.

The repaired recipe compiles, assembles, links and runs with the current host
tools. The classic assembly source and resulting object are unchanged; object
SHA-256 is
`0d8fe759b3d6a4d4668cf0b25b0f667be3df4743fcde9dde355124724cc301a1`.
The selected replacement XMIT is 1,440 bytes, SHA-256
`b6ad17e6a3c445ffcb97392b106faf6dec90cdafca28f9fcc406a8754924cb26`;
its independently extracted native stream is 1,064 bytes, SHA-256
`443c88e9fa04a232af83010f7ea7c98874a39f4475217660b916c531452ab77c`.
The load directory reports MFFLT31/31/ANY. The source still contains only the
intentional invalid instruction and allocates no C stack, heap or files.

Strictly verify this amendment's delta manifest and the unchanged original
pack/supplement before transfer. Require exact binary transfer readback,
successful restore and native LISTDS member MFFLT31 before invoking it only
through the reviewed IKJEFTSR CALL path. Preserve the previous failed delivery
receipt. Replay native TSO nonzero/fault recovery, caller-file survival and
subsequent successful TIME; arbitrary CMS child-abend handling remains outside
this control. No managed-image adoption or publication is implied.
