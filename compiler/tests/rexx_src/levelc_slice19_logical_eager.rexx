options levelc
calls = 0
say 0 & tick()
say 'calls' calls
say 1 | tick()
say 'calls' calls
say 1 & tick()
say 'calls' calls
say 0 | tick()
say 'calls' calls
flag = 1
say flag & mutateone()
say 'flag' flag
flag = 1
say flag | mutatezero()
say 'flag' flag
say (0 & tick()) | tick()
say 'calls' calls
exit
tick:
procedure expose calls
calls = calls + 1
return 1
mutateone:
procedure expose flag
flag = 0
return 1
mutatezero:
procedure expose flag
flag = 0
return 0
