options levelc
do 1
  call on error name caught
  leave
  say 'unexpected'
end
say 'after-leave'
call routine
say 'after-return'
call on error name caught
exit
caught:
  say 'trap=' || sigl || '|' || condition('C') || '|' || condition('D')
  return
routine:
  call on error name caught
  return
