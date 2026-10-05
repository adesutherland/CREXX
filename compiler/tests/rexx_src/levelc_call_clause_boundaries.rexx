options levelc
flag = 1
call on error name caught
if flag then say 'if-body'
call off error
flag = 1
call on error name caught
select
  when flag then say 'when-body'
  otherwise say 'unexpected'
end
call off error
times = 1
call on error name caught
do times
  say 'do-body'
end
call off error
exit
caught:
  flag = 0
  times = 0
  say 'trap=' || sigl || '|' || condition('C') || '|' || condition('D')
  return
