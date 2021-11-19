/* D2C */
options levelb
say "Look for D2C OK"
/* These from the Rexx book. */
if td2c(9)      \= '09'  then say 'failed in test 1 '
if td2c(129)    \= '81'  then say 'failed in test 2 '
if td2c(129,1)  \= '81'  then say 'failed in test 3 '
if td2c(129,2)  \= '0081' then say 'failed in test 4 '
if td2c(257,1)  \= '01'   then say 'failed in test 5 '
if td2c(-127,1) \= '81'   then say 'failed in test 6 '
if td2c(-127,2) \= 'FF81' then say 'failed in test 7 '
if td2c(-1,4) \= 'FFFFFFFF' then say 'failed in test 8 '
if td2c(12,0) \= '' then say 'failed in test 9 '
/* These from Mark Hessling. */
if td2c(127) \= "7F"  then say 'failed in test 10 '
if td2c(128) \= "80"  then say 'failed in test 11 '
if td2c(129) \= "81"  then say 'failed in test 12 '
if td2c(1) \= "01"  then say 'failed in test 13 '
if td2c(-1,1) \= "FF"  then say 'failed in test 14 '
if td2c(-127,1) \= "81"  then say 'failed in test 15 '
if td2c(-128,1) \= "80"  then say 'failed in test 16 '
if td2c(-129,1) \= "7F" then say 'failed in test 17 '
if td2c(-1,2) \= "FFFF" then say 'failed in test 18 '
if td2c(-127,2) \= "FF81" then say 'failed in test 19 '
if td2c(-128,2) \= "FF80" then say 'failed in test 20 '
if td2c(-129,2) \= "FF7F" then say 'failed in test 21 '
if td2c(129,0) \= "" then say 'failed in test 22 '
if td2c(129,1) \= "81" then say 'failed in test 23 '
if td2c(256+129,2) \= "0181" then say 'failed in test 24 '
if td2c(256*256+256+129,3) \= "010181" then say 'failed in test 25 '
say "D2C OK"
return

td2c: procedure = .string
  arg hex = .int, len=-1
  slen=0
  str=d2c(hex,len)
  str2=c2x(str)  /* 2 half bytes translated into 2 bytes */
  assembler strlen slen,str2
  if slen//2=1 then str2='0'str2
return str2



x2d: procedure = .int
  arg hex = .string, slen = -1

x2c: procedure = .string
  arg hex = .string

x2b: procedure = .string
  arg hex = .string, slen = -1

d2b: procedure = .string
  arg int1 = .int, slen = -1

d2x: procedure = .string
  arg int1 = .int, slen = -1

d2c: procedure = .string
  arg int1 = .int, slen = -1

c2x: procedure = .string
  arg string = .string


