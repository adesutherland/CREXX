/* X2B */
options levelb
say "Look for X2B OK"
/* These from the Rexx book. */
if x2b('C3') \= '11000011' then say 'failed in test 1 '
if x2b('7') \= '0111' then say 'failed in test 2 '
if x2b('1 C1') \= '000111000001' then say 'failed in test 3 '
if x2b(c2x('C3'x)) \= '11000011' then say 'failed in test 4 '
if x2b(d2x('129')) \= '10000001' then say 'failed in test 5 '
if x2b(d2x('12')) \= '1100' then say 'failed in test 6 '
/* These from Mark Hessling. */
if x2b("416263") \= "010000010110001001100011" then say 'failed in test 7 '
if x2b("DeadBeef") \= "11011110101011011011111011101111" then say 'failed in test 8 '
if x2b("1 02 03") \= "00010000001000000011" then say 'failed in test 9 '
if x2b("102 03") \= "00010000001000000011" then say 'failed in test 10 '
if x2b("102") \= "000100000010" then say 'failed in test 11 '
if x2b("11 2F") \= "0001000100101111" then say 'failed in test 12 '
if x2b("") \= "" then say 'failed in test 13 '
say "X2B OK"

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

