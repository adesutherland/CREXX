/* rexx */
options levelb

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
