#!/usr/local/crexx/rexx.sh
/* Address testbed */
options levelb

arg args = .string[]

do i = 1 to args.0
  say "Arg" arg.i
