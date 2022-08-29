/* rexx */
options levelb
import rxfnsb

/* say "Hex to char" */
/* say x2c('616263') 'abc' */
/* say x2c('343536') '456' */
/* say "Decimal to Hex" */
/* say d2x(-128) */
/* say d2x(128) */
/* say d2x(-3967) */
/* say d2x(32640) */
/* say D2X(9)        '->     9' */
/* say D2X(129)      '->    81' */
/* say D2X(129,1)    '->     1' */
/* say D2X(129,2)    '->    81' */
/* say D2X(129,4)    '->    0081' */
/* say D2X(257,2)    '->    01' */
/* say D2X(-127,2)   '->    81' */
/* say D2X(-127,4)   '->    FF81' */
/* say "'"D2X(12,0)"'" '->    ""' */
/* say "Decimal to Bit" */
/* say d2b(-128) */
/* say d2b(128) */
/* say d2b(-3967) */
/* say d2b(32640) */
/* say "Hex to Bit" */
/* say x2b( 'ff80') */
/* say x2b( '7f80') */
/* say x2b( 'ff80') */
/* say x2b( 'ff80') */
/* say x2b('71') */
/* say x2b('81') */
/* say x2b('F081') */
/* say x2b('F081') */
/* say x2b('F081') */
/* say x2b('F081') */
/* say x2b('0031') */
errors=0
/* X2D tests  */
if x2d( 'ff80',4) \=-128 then do
  errors=errors+1
  say "XD2 failed in test "0
end
if x2d( '7f80',4) \=32640 then do
  errors=errors+1
  say "XD2 failed in test 1"
end
if x2d( 'ff80',2) \=-128 then do
  errors=errors+1
  say "XD2 failed in test 2"
end
if x2d( 'ff80',5) \=65408 then do
  errors=errors+1
  say "XD2 failed in test 3"
end
if x2d('81',2) \= -127 then do
  errors=errors+1
  say "XD2 failed in test 4"
end
if x2d('81',4) \= 129 then do
  errors=errors+1
  say "XD2 failed in test 5"
end
if x2d('F081',4) \= -3967 then do
  errors=errors+1
  say "XD2 failed in test 6"
end
if x2d('F081',3) \= 129 then do
  errors=errors+1
  say "XD2 failed in test 7"
end
if x2d('F081',2) \= -127 then do
  errors=errors+1
  say "XD2 failed in test 8"
end
if x2d('F081',1) \= 1 then do
  errors=errors+1
  say "XD2 failed in test 9"
end
if x2d('0031',0) \= 0 then do
  errors=errors+1
  say "XD2 failed in test 10"
  /* say 'Done x2d.rexx' */
  /* say "test D2X" */
  /* xc=d2c(7) */
  /* say 1 c2x(xc) '07x' */
  /* xc=d2c(129) */
  /* say 2 c2x(xc) '81x' */
  /* say 'Done d2x.rexx' */
  /* say 3 "'"c2x('')"'" "''" */
  /* say 'Done d2x.rexx' */
  /* X2D */
  say "Look for X2D OK"
  /* These from the Rexx book. */
end
if x2d('0E') \= 14 then do
  errors=errors+1
  say 'failed in test 11 '
end
if x2d('81') \= 129 then do
  errors=errors+1
  say 'failed in test 12 x2d('81')' x2d('81') 'but must be 129'
end
if x2d('F81') \= 3969 then do
  errors=errors+1
  say 'failed in test 13 x2d('F81')' x2d('F81') 'but must be 3969'
end
if x2d('FF81') \= 65409 then do
  errors=errors+1
  say 'failed in test 14 x2d('FF81')' x2d('FF81') 'but must be 65409'
end
if x2d('c6 f0'x) \= 240 then do
  errors=errors+1
  say 'failed in test 15 x2d('c6 f0'x)' x2d('c6 f0'x) 'but must be 240 but gives error because of space in c6 f0'
end
if x2d('F0') \= 240 then do
  errors=errors+1
  say 'failed in test 16 x2d('F0')' x2d('F0') 'but must be 240'
end
if x2d('81',2) \= -127 then do
  errors=errors+1
  say 'failed in test 17 '
end
if x2d('81',4) \= 129 then do
  errors=errors+1
  say 'failed in test 18 '
end
if x2d('F081',4) \= -3967 then do
  errors=errors+1
  say 'failed in test 19 '
end
if x2d('F081',3) \= 129 then do
  errors=errors+1
  say 'failed in test 20 '
end
if x2d('F081',2) \= -127 then do
  errors=errors+1
  say 'failed in test 21 '
end
if x2d('F081',1) \= 1 then do
  errors=errors+1
  say 'failed in test 22 '
end
if x2d('0031',0) \= 0 then do
  errors=errors+1
  say 'failed in test 23 '
  /* These from Mark Hessling. */
end
if x2d( 'ff80', 2) \= "-128" then do
  errors=errors+1
  say 'failed in test 24 '
end
if x2d( 'ff80', 1) \= "0" then do
  errors=errors+1
  say 'failed in test 25 '
end
if x2d( 'ff 80', 1) \= "0" then do
  errors=errors+1
  say 'failed in test 26 '
end
if x2d( '' ) \= "0" then do
  errors=errors+1
  say 'failed in test 27 '
end
if x2d( '101' ) \= "257" then do
  errors=errors+1
  say 'failed in test 28 '
end
if x2d( 'ff' ) \= "255" then do
  errors=errors+1
  say 'failed in test 29 x2d( 'ff' )' x2d( 'ff' ) 'but must be 255'
end
if x2d( 'ffff') \= "65535" then do
  errors=errors+1
  say 'failed in test 30 x2d( 'ffff')' x2d( 'ffff') 'but must be 65535'
end
if x2d( 'ffff', 2) \= "-1" then do
  errors=errors+1
  say 'failed in test 31 '
end
if x2d( 'ffff', 1) \= "-1" then do
  errors=errors+1
  say 'failed in test 32 '
end
if x2d( 'fffe', 2) \= "-2" then do
  errors=errors+1
  say 'failed in test 33 '
end
if x2d( 'fffe', 1) \= "-2" then do
  errors=errors+1
  say 'failed in test 34 '
end
if x2d( 'ffff', 4) \= "-1" then do
  errors=errors+1
  say 'failed in test 35 '
end
if x2d( 'ffff', 2) \= "-1" then do
  errors=errors+1
  say 'failed in test 36 '
end
if x2d( 'fffe', 4) \= "-2" then do
  errors=errors+1
  say 'failed in test 37 '
end
if x2d( 'fffe', 2) \= "-2" then do
  errors=errors+1
  say 'failed in test 38 '
end
if x2d( 'ffff', 3) \= "-1" then do
  errors=errors+1
  say 'failed in test 39 '
end
if x2d( '0fff') \= "4095" then do
  errors=errors+1
  say 'failed in test 30 '
end
if x2d( '0fff', 4) \= "4095" then do
  errors=errors+1
  say 'failed in test 41 '
end
if x2d( '0fff', 3) \= "-1" then do
  errors=errors+1
  say 'failed in test 42 '
end
if x2d( '07ff') \= "2047" then do
  errors=errors+1
  say 'failed in test 43 '
end
if x2d( '07ff', 4) \= "2047" then do
  errors=errors+1
  say 'failed in test 44 '
end
if x2d( '07ff', 3) \= "2047" then do
  errors=errors+1
  say 'failed in test 45 '
end
if x2d( 'ff7f', 1) \= "-1" then do
  errors=errors+1
  say 'failed in test 46 '
end
if x2d( 'ff7f', 2) \= "127" then do
  errors=errors+1
  say 'failed in test 47 '
end
if x2d( 'ff7f', 3) \= "-129" then do
  errors=errors+1
  say 'failed in test 48 '
end
if x2d( 'ff7f', 4) \= "-129" then do
  errors=errors+1
  say 'failed in test 49 '
end
if x2d( 'ff7f', 5) \= "65407" then do
  errors=errors+1
  say 'failed in test 50 '
end
if x2d( 'ff80', 1) \= "0" then do
  errors=errors+1
  say 'failed in test 51 '
end
if x2d( 'ff80', 2) \= "-128" then do
  errors=errors+1
  say 'failed in test 52 '
end
if x2d( 'ff80', 3) \= "-128" then do
  errors=errors+1
  say 'failed in test 53 '
end
if x2d( 'ff80', 4) \= "-128" then do
  errors=errors+1
  say 'failed in test 54 x2d( 'ff80', 4)' x2d( 'ff80', 4) 'but must be -128'
end
if x2d( 'ff80', 5) \= "65408" then do
  errors=errors+1
  say 'failed in test 55 '
end
if x2d( 'ff81', 1) \= "1" then do
  errors=errors+1
  say 'failed in test 56 x2d( 'ff81', 1) ' x2d( 'ff81', 1) 'but must be 1'
end
if x2d( 'ff81', 2) \= "-127" then do
  errors=errors+1
  say 'failed in test 57 '
end
if x2d( 'ff81', 3) \= "-127" then do
  errors=errors+1
  say 'failed in test 58 '
end
if x2d( 'ff81', 4) \= "-127" then do
  errors=errors+1
  say 'failed in test 59 '
end
if x2d( 'ff81', 5) \= "65409" then do
  errors=errors+1
  say 'failed in test 60 '
end
if x2d( 'ffffffffffff', 12) \= "-1" then do
  errors=errors+1
  say 'failed in test 61 '
  /* These from SCXB
     END
     IFA4 */
  /* 
     end
     if X2D((00000000000000001+1-0.000000)) \= '2' then do
     errors=errors+1
     say 'failed in test 52 ' */
  /* 
     end
     if X2D((1&1|0=22*33)) \= '1' then do
     errors=errors+1
     say 'failed in test 53 ' */
  /* 
     end
     if X2D((99/3+10*126-(33||2)//5-1099)) \= '402' then do
     errors=errors+1
     say 'failed in test 54 ' */
  /* 
     end
     if X2D(ABS((99/3+10*126-(33||2)//5-1099))) \= '402' then do
     errors=errors+1
     say 'failed in test 55 ' */
  /* 
     end
     if X2D(ABS((1&1|0=22*33))) \= '1' then do
     errors=errors+1
     say 'failed in test 56 ' */
  /* 
     end
     if X2D(ABS((00000000000000001+1-0.000000))) \= '2' then do
     errors=errors+1
     say 'failed in test 57 ' */
  /* 
     end
     if X2D(ABS(COPIES(0,249)||1)) \= '1' then do
     errors=errors+1
     say 'failed in test 58 ' */
  /* 
     end
     if X2D(ABS(RIGHT(LEFT(REVERSE(321),2),REVERSE(LEFT(123,ABS(-1)))))) \= '2' then do
     errors=errors+1
     say 'failed in test 59 ' */
  /* 
     end
     if X2D(COPIES(0,249)||1) \= '1' then do
     errors=errors+1
     say 'failed in test 60 ' */
  /* 
     end
     if X2D(RIGHT(LEFT(REVERSE(321),2),REVERSE(LEFT(123,ABS(-1))))) \= '2' then do
     errors=errors+1
     say 'failed in test 61 ' */
  /* These from SCBx2d1 */
end
if X2D('') \= '0' then do
  errors=errors+1
  say 'failed in test 62 '
end
if x2d(''X) \= '0' then do
  errors=errors+1
  say 'failed in test 63 '
end
if X2D('a') \= '10' then do
  errors=errors+1
  say 'failed in test 64 X2D('a')' X2D('a') 'but must be 10'
end
if X2D('0f') \= '15' then do
  errors=errors+1
  say 'failed in test 65 '
end
if x2D('80') \= '128' then do
  errors=errors+1
  say 'failed in test 66 x2D('80')' x2D('80') 'but must be 128'
end
if x2d('765') \= '1893' then do
  errors=errors+1
  say 'failed in test 67 '
  /* BREXX Does not support large # digits
     Numeric Digits 1000
     end
     if x2D("eeeeeeeeeeeeeeeeeeeeeeeee") \=,
     '1183140560213014108063589658350' then do
     errors=errors+1
     say 'failed in test 68 '
     Numeric Digits
   */
end
if x2d(01234) \= '4660' then do
  errors=errors+1
  say 'failed in test 68 '
end
if x2d(1E2) \= '482' then do
  errors=errors+1
  say 'failed in test 69 '
end
if x2d(+1E+2) \= '256' then do
  errors=errors+1
  say 'failed in test 70 '
end
if x2d(+.1E2) \= '16' then do
  errors=errors+1
  say 'failed in test 71 '
  /* Signal Off Novalue */
end
if x2d(baba) \= '47802' then do
  errors=errors+1
  say 'failed in test 72 x2d(baba)' x2d(baba) 'but must be 47802'
end
if X2D(1 + 1E+2 ) \= '257' then do
  errors=errors+1
  say 'failed in test 73 '
end
if X2D('',0) \= '0' then do
  errors=errors+1
  say 'failed in test 74 '
end
if X2D('',12) \= '0' then do
  errors=errors+1
  say 'failed in test 75 '
end
if X2D('abc',0) \= '0' then do
  errors=errors+1
  say 'failed in test 76 '
end
if X2D('abc',1) \= '-4' then do
  errors=errors+1
  say 'failed in test 77 '
end
if X2D('abc',3) \= '-1348' then do
  errors=errors+1
  say 'failed in test 78 '
end
if X2D('abc',5) \= '2748' then do
  errors=errors+1
  say 'failed in test 79 '
end
if X2D('abc',12345) \= '2748' then do
  errors=errors+1
  say 'failed in test 80 '
end
if x2d(1+3,1+3) \= '4' then do
  errors=errors+1
  say 'failed in test 81 '
end
if x2D(256+12,10+2) \= '616' then do
  errors=errors+1
  say 'failed in test 82 '
end
if x2d('12',987654321) \= '18' then do
  errors=errors+1
  say 'failed in test 83 '
end
if d2x(X2D('12345')) \= '12345' then do
  errors=errors+1
  say 'failed in test 84 '
end
return errors<>0
