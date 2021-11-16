/* rexx */
options levelb
/* X2C */
say "Look for X2C OK"
/* These from the Rexx book. */
/* if x2c('F7F2 A2') \= '72s' then say 'failed in test EBCDIC */
/* if x2c('F7F2a2') \= '72s' then say 'failed in test EBCDIC */
if x2c('F') \= '0F'x then say 'failed in test 3 '
/* These from Mark Hessling. */
/* if x2c("416263") \= "Abc" then say 'failed in test 4 ' probably also EBCDIC */
if x2c("DeadBeef") \= "deadbeef"x then say 'failed in test 5 '
if x2c("1 02 03") \= "010203"x then say 'failed in test 6 '
if x2c("11 0222 3333 044444") \= "1102223333044444"x then say 'failed in test 7 '
if x2c("") \= "" then say 'failed in test 8 '
if x2c("2") \= "02"x then say 'failed in test 9 '
if x2c("1 02 03") \= "010203"x then say 'failed in test 10 '
say "X2C OK"

return

x2d: procedure = .int
  arg expose hex = .string, slen = -1
x2c: procedure = .string
  arg expose hex = .string

x2b: procedure = .string
  arg expose hex = .string, slen = -1

d2b: procedure = .string
  arg expose int1 = .int, slen = -1

d2x: procedure = .string
  arg expose int1 = .int, slen = -1

d2c: procedure = .string
  arg expose int1 = .int, slen = -1

c2x: procedure = .string
  arg expose string = .string

