options levelc
address system 'echo hi'
address value environment with output stream out replace
arg first second
call subroutine 1, 2
call on error name handler
call off halt
drop alpha beta stem.tail
exit 0
interpret "say 'dynamic'"
nop
numeric digits 9
numeric form scientific
numeric form value formSetting
numeric fuzz 0
pull first second
push 'front'
queue 'back'
return 'done'
signal handler
signal on novalue name handler
signal off syntax
signal value handler
trace normal
trace value traceSetting
handler: nop
subroutine: procedure expose shared stem.tail
return
