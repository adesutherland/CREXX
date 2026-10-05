options levelc
phase = 0
call é
say 'main'
signal 'É'
say 'skip'
é:
say 'first'
É:
say 'second'
if phase = 0 then do
  phase = 1
  signal value 'é'
end
return
