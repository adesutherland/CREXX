/* D2X */
options levelb
say "Look for D2X OK"
/* These from the Rexx book. */
if d2x(9) \= '9' then say 'failed in test 1 '
if d2x(129) \= '81' then say 'failed in test 2 '
if d2x(129,1) \= '1' then say 'failed in test 3 '
if d2x(129,2) \= '81' then say 'failed in test 4 '
if d2x(129,4) \= '0081' then say 'failed in test 5 '
if d2x(257,2) \= '01' then say 'failed in test 6 '
if d2x(-127,2) \= '81' then say 'failed in test 7 '
if d2x(-127,4) \= 'FF81' then say 'failed in test 8 '
if d2x(12,0) \= '' then say 'failed in test 9 '
/* These from Mark Hessling. */
/* if d2x(0) \= "" then say 'failed in test 10 ' */
if d2x(127) \= "7F" then say 'failed in test 11 '
if d2x(128) \= "80" then say 'failed in test 12 '
if d2x(129) \= "81" then say 'failed in test 13 '
if d2x(1) \= "1" then say 'failed in test 14 '
if d2x(-1,2) \= "FF" then say 'failed in test 15 '
if d2x(-127,2) \= "81" then say 'failed in test 16 '
if d2x(-128,2) \= "80" then say 'failed in test 17 '
if d2x(-129,2) \= "7F" then say 'failed in test 18 '
if d2x(-1,3) \= "FFF" then say 'failed in test 19 '
if d2x(-127,3) \= "F81" then say 'failed in test 20 '
if d2x(-128,4) \= "FF80" then say 'failed in test 21 '
if d2x(-129,5) \= "FFF7F" then say 'failed in test 22 '
if d2x(129,0) \= "" then say 'failed in test 23 '
if d2x(129,2) \= "81" then say 'failed in test 24 '
if d2x(256+129,4) \= "0181" then say 'failed in test 25 '
if d2x(256*256+256+129,6) \= "010181" then say 'failed in test 26 '
say "D2X OK"



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


