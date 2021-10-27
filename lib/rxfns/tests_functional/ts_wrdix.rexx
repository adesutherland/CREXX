/* WORDINDEX */
options levelb
  say "Look for WORDINDEX OK"
/* These from the Rexx book. */
  if wordindex('Now is the time',3) \= 8  then say 'failed in test          1 '
  if wordindex('Now is the time',6) \= 0  then say 'failed in test          2 '
/* These from Mark Hessling. */
  if wordindex('This is certainly a test',1) \=  '1'  then say 'failed in test          3 '
  if wordindex('  This is certainly a test',1) \=  '3'  then say 'failed in test          4 '
  if wordindex('This   is certainly a test',1) \=  '1'  then say 'failed in test          5 '
  if wordindex('  This   is certainly a test',1) \=  '3'  then say 'failed in test          6 '
  if wordindex('This is certainly a test',2) \=  '6'  then say 'failed in test          7 '
  if wordindex('This   is certainly a test',2) \=  '8'  then say 'failed in test          8 '
  if wordindex('This is   certainly a test',2) \=  '6'  then say 'failed in test          9 '
  if wordindex('This   is   certainly a test',2) \=  '8'  then say 'failed in test         10 '
  if wordindex('This is certainly a test',5) \=  '21'   then say 'failed in test         11 '
  if wordindex('This is certainly a   test',5) \=  '23'  then say 'failed in test         12 '
  if wordindex('This is certainly a test  ',5) \=  '21'  then say 'failed in test         13 '
  if wordindex('This is certainly a test  ',6) \=  '0'   then say 'failed in test         14 '
  if wordindex('This is certainly a test',6) \=  '0'     then say 'failed in test         15 '
  if wordindex('This is certainly a test',7) \=  '0'     then say 'failed in test         16 '
  if wordindex('This is certainly a test  ',7) \=  '0'    then say 'failed in test         17 '
  say "WORDINDEX OK"

  /* function prototype */
  wordindex: procedure = .int
  arg string1 = .string, int2 = .int



  