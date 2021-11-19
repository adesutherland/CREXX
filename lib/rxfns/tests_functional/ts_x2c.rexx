/* rexx */
options levelb
/* X2C */
say "Look for X2C OK"
/* These from the Rexx book. */
/* if x2c('F7F2 A2') \= '72s' then say 'failed in test EBCDIC */
/* if x2c('F7F2a2') \= '72s' then say 'failed in test EBCDIC */
if tx2c('F') \= '0F' then say 'failed in test 3 '
   else say 'test 3 successful: 0F'
/* These from Mark Hessling. */
/* if x2c("416263") \= "Abc" then say 'failed in test 4 ' probably also EBCDIC */
if tx2c("DeadBeef") \= "deadbeef" then say 'failed in test 5 '
    else say 'test 5 successful: deadbeef'
if tx2c("1 02 03") \= "010203" then say 'failed in test 6 '
 else say 'test 6 successful: 010203'
if tx2c("11 0222 3333 044444") \= "1102223333044444" then say 'failed in test 7 '
 else say 'test 7 successful: 1102223333044444'
if tx2c("") \= "" then say 'failed in test 8 '
   else say 'test 8 successful: ""'
if tx2c("2") \= "02" then say 'failed in test 9 '
  else say 'test 9 successful: 02'
if tx2c("1 02 03") \= "010203" then say 'failed in test 10 '
   else say 'test 10 successful: 010203'
say "X2C OK"

return

tx2c: procedure = .string
  arg hex = .string
  slen=0
  str=x2c(hex)
  str2=c2x(str)  /* 2 half bytes translated into 2 bytes */
  assembler strlen slen,str2
  if slen//2=1 then str2='0'str2
  say "'"str2"'"
return str2


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

