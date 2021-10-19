/* rexx */
options levelb

say "decimal to Hex"
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
return

x2d: procedure = .int
  arg expose hex = .string, slen = -1

x2b: procedure = .string
  arg expose hex = .string, slen = -1

d2b: procedure = .string
  arg expose int1 = .int, slen = -1

d2x: procedure = .string
  arg expose int1 = .int, slen = -1

