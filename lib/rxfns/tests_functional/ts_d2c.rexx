/* D2C */
options levelb
say "Look for D2C OK"
/* These from the Rexx book. */
if d2c(9) \= '09'x then say 'failed in test 1 '
if d2c(129) \= '81'x then say 'failed in test 2 '
if d2c(129,1) \= '81'x then say 'failed in test 3 '
if d2c(129,2) \= '0081'x then say 'failed in test 4 '
if d2c(257,1) \= '01'x then say 'failed in test 5 '
if d2c(-127,1) \= '81'x then say 'failed in test 6 '
if d2c(-127,2) \= 'FF81'x then say 'failed in test 7 '
if d2c(-1,4) \= 'FFFFFFFF'x then say 'failed in test 8 '
if d2c(12,0) \= '' then say 'failed in test 9 '
/* These from Mark Hessling. */
if d2c(127) \= "7f"x then say 'failed in test 10 '
if d2c(128) \= "80"x then say 'failed in test 11 '
if d2c(129) \= "81"x then say 'failed in test 12 '
if d2c(1) \= "01"x then say 'failed in test 13 '
if d2c(-1,1) \= "FF"x then say 'failed in test 14 '
if d2c(-127,1) \= "81"x then say 'failed in test 15 '
if d2c(-128,1) \= "80"x then say 'failed in test 16 '
if d2c(-129,1) \= "7F"x then say 'failed in test 17 '
if d2c(-1,2) \= "FFFF"x then say 'failed in test 18 '
if d2c(-127,2) \= "FF81"x then say 'failed in test 19 '
if d2c(-128,2) \= "FF80"x then say 'failed in test 20 '
if d2c(-129,2) \= "FF7F"x then say 'failed in test 21 '
if d2c(129,0) \= "" then say 'failed in test 22 '
if d2c(129,1) \= "81"x then say 'failed in test 23 '
if d2c(256+129,2) \= "0181"x then say 'failed in test 24 '
if d2c(256*256+256+129,3) \= "010181"x then say 'failed in test 25 '
say "D2C OK"



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


