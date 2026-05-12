options levelc
address system 'echo hi'
arg first second
call subroutine 1, 2
drop alpha beta stem.tail
exit 0
interpret "say 'dynamic'"
nop
numeric digits 9
numeric form scientific
numeric fuzz 0
procedure expose shared stem.tail
pull first second
push 'front'
queue 'back'
return 'done'
signal handler
trace normal
handler: nop
subroutine: return
