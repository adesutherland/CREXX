/* rexx */
options levelb
say "Hex to char"
say x2c('616263') 'abc'
say x2c('343536') '456'

say "Decimal to Hex"
say d2x(-128)
say d2x(128)
say d2x(-3967)
say d2x(32640)

say D2X(9)        '->     9'
say D2X(129)      '->    81'
say D2X(129,1)    '->     1'
say D2X(129,2)    '->    81'
say D2X(129,4)    '->    0081'
say D2X(257,2)    '->    01'
say D2X(-127,2)   '->    81'
say D2X(-127,4)   '->    FF81'
say "'"D2X(12,0)"'" '->    ""'

say "Decimal to Bit"
say d2b(-128)
say d2b(128)
say d2b(-3967)
say d2b(32640)

say "Hex to Bit"
say x2b( 'ff80')
say x2b( '7f80')
say x2b( 'ff80')
say x2b( 'ff80')
say x2b('71')
say x2b('81')
say x2b('F081')
say x2b('F081')
say x2b('F081')
say x2b('F081')
say x2b('0031')


/* X2D tests  */
if x2d( 'ff80',4) \=-128 then say "error in "0
if x2d( '7f80',4) \=32640 then say "error in "1
if x2d( 'ff80',2) \=-128 then say "error in "2
if x2d( 'ff80',5) \=65408 then say "error in "3
if x2d('81',2) \= -127 then say "error in "4
if x2d('81',4) \= 129 then say "error in "5
if x2d('F081',4) \= -3967 then say "error in "6
if x2d('F081',3) \= 129 then say "error in "7
if x2d('F081',2) \= -127 then say "error in "8
if x2d('F081',1) \= 1 then say "error in "9
if x2d('0031',0) \= 0 then say "error in "10

say 'Done x2d.rexx'

say "test D2X"
xc=d2c(7)
say 1 c2x(xc) '07x'
xc=d2c(129)
say 2 c2x(xc) '81x'
say 'Done d2x.rexx'
say 3 "'"c2x('')"'" "''"
say 'Done d2x.rexx'

/* X2D */
say "Look for X2D OK"
/* These from the Rexx book. */
if x2d('0E') \= 14 then say 'failed in test 1 '
if x2d('81') \= 129 then say 'failed in test 2 '
if x2d('F81') \= 3969 then say 'failed in test 3 '
if x2d('FF81') \= 65409 then say 'failed in test 4 '
if x2d('c6 f0'x) \= 240 then say 'failed in test 5 '
if x2d('F0') \= 240 then say 'failed in test 6 '
if x2d('81',2) \= -127 then say 'failed in test 7 '
if x2d('81',4) \= 129 then say 'failed in test 8 '
if x2d('F081',4) \= -3967 then say 'failed in test 9 '
if x2d('F081',3) \= 129 then say 'failed in test 10 '
if x2d('F081',2) \= -127 then say 'failed in test 11 '
if x2d('F081',1) \= 1 then say 'failed in test 12 '
if x2d('0031',0) \= 0 then say 'failed in test 13 '
/* These from Mark Hessling. */
if x2d( 'ff80', 2) \= "-128" then say 'failed in test 14 '
if x2d( 'ff80', 1) \= "0" then say 'failed in test 15 '
if x2d( 'ff 80', 1) \= "0" then say 'failed in test 16 '
if x2d( '' ) \= "0" then say 'failed in test 17 '
if x2d( '101' ) \= "257" then say 'failed in test 18 '
if x2d( 'ff' ) \= "255" then say 'failed in test 19 '
if x2d( 'ffff') \= "65535" then say 'failed in test 20 '
if x2d( 'ffff', 2) \= "-1" then say 'failed in test 21 '
if x2d( 'ffff', 1) \= "-1" then say 'failed in test 22 '
if x2d( 'fffe', 2) \= "-2" then say 'failed in test 23 '
if x2d( 'fffe', 1) \= "-2" then say 'failed in test 24 '
if x2d( 'ffff', 4) \= "-1" then say 'failed in test 25 '
if x2d( 'ffff', 2) \= "-1" then say 'failed in test 26 '
if x2d( 'fffe', 4) \= "-2" then say 'failed in test 27 '
if x2d( 'fffe', 2) \= "-2" then say 'failed in test 28 '
if x2d( 'ffff', 3) \= "-1" then say 'failed in test 29 '
if x2d( '0fff') \= "4095" then say 'failed in test 30 '
if x2d( '0fff', 4) \= "4095" then say 'failed in test 31 '
if x2d( '0fff', 3) \= "-1" then say 'failed in test 32 '
if x2d( '07ff') \= "2047" then say 'failed in test 33 '
if x2d( '07ff', 4) \= "2047" then say 'failed in test 34 '
if x2d( '07ff', 3) \= "2047" then say 'failed in test 35 '
if x2d( 'ff7f', 1) \= "-1" then say 'failed in test 36 '
if x2d( 'ff7f', 2) \= "127" then say 'failed in test 37 '
if x2d( 'ff7f', 3) \= "-129" then say 'failed in test 38 '
if x2d( 'ff7f', 4) \= "-129" then say 'failed in test 39 '
if x2d( 'ff7f', 5) \= "65407" then say 'failed in test 40 '
if x2d( 'ff80', 1) \= "0" then say 'failed in test 41 '
if x2d( 'ff80', 2) \= "-128" then say 'failed in test 42 '
if x2d( 'ff80', 3) \= "-128" then say 'failed in test 43 '
if x2d( 'ff80', 4) \= "-128" then say 'failed in test 44 '
if x2d( 'ff80', 5) \= "65408" then say 'failed in test 45 '
if x2d( 'ff81', 1) \= "1" then say 'failed in test 46 '
if x2d( 'ff81', 2) \= "-127" then say 'failed in test 47 '
if x2d( 'ff81', 3) \= "-127" then say 'failed in test 48 '
if x2d( 'ff81', 4) \= "-127" then say 'failed in test 49 '
if x2d( 'ff81', 5) \= "65409" then say 'failed in test 50 '
if x2d( 'ffffffffffff', 12) \= "-1" then say 'failed in test 51 '
/* These from SCXBIFA4 */
/* if X2D((00000000000000001+1-0.000000)) \= '2' then say 'failed in test 52 ' */
/* if X2D((1&1|0=22*33)) \= '1' then say 'failed in test 53 ' */
/* if X2D((99/3+10*126-(33||2)//5-1099)) \= '402' then say 'failed in test 54 ' */
/* if X2D(ABS((99/3+10*126-(33||2)//5-1099))) \= '402' then say 'failed in test 55 ' */
/* if X2D(ABS((1&1|0=22*33))) \= '1' then say 'failed in test 56 ' */
/* if X2D(ABS((00000000000000001+1-0.000000))) \= '2' then say 'failed in test 57 ' */
/* if X2D(ABS(COPIES(0,249)||1)) \= '1' then say 'failed in test 58 ' */
/* if X2D(ABS(RIGHT(LEFT(REVERSE(321),2),REVERSE(LEFT(123,ABS(-1)))))) \= '2' then say 'failed in test 59 ' */
/* if X2D(COPIES(0,249)||1) \= '1' then say 'failed in test 60 ' */
/* if X2D(RIGHT(LEFT(REVERSE(321),2),REVERSE(LEFT(123,ABS(-1))))) \= '2' then say 'failed in test 61 ' */
/* These from SCBx2d1 */
if X2D('') \= '0' then say 'failed in test 62 '
if x2d(''X) \= '0' then say 'failed in test 63 '
if X2D('a') \= '10' then say 'failed in test 64 '
if X2D('0f') \= '15' then say 'failed in test 65 '
if x2D('80') \= '128' then say 'failed in test 66 '
if x2d('765') \= '1893' then say 'failed in test 67 '
/* BREXX Does not support large # digits
Numeric Digits 1000
if x2D("eeeeeeeeeeeeeeeeeeeeeeeee") \=,
'1183140560213014108063589658350' then say 'failed in test 68 '
Numeric Digits
*/
if x2d(01234) \= '4660' then say 'failed in test 69 '
if x2d(1E2) \= '482' then say 'failed in test 70 '
if x2d(+1E+2) \= '256' then say 'failed in test 71 '
if x2d(+.1E2) \= '16' then say 'failed in test 72 '
/* Signal Off Novalue */
if x2d(baba) \= '47802' then say 'failed in test 73 '
if X2D(1 + 1E+2 ) \= '257' then say 'failed in test 74 '
if X2D('',0) \= '0' then say 'failed in test 75 '
if X2D('',12) \= '0' then say 'failed in test 76 '
if X2D('abc',0) \= '0' then say 'failed in test 77 '
if X2D('abc',1) \= '-4' then say 'failed in test 78 '
if X2D('abc',3) \= '-1348' then say 'failed in test 79 '
if X2D('abc',5) \= '2748' then say 'failed in test 80 '
if X2D('abc',12345) \= '2748' then say 'failed in test 81 '
if x2d(1+3,1+3) \= '4' then say 'failed in test 82 '
if x2D(256+12,10+2) \= '616' then say 'failed in test 83 '
if x2d('12',987654321) \= '18' then say 'failed in test 84 '
if d2x(X2D('12345')) \= '12345' then say 'failed in test 85 '
say "X2D OK"

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

