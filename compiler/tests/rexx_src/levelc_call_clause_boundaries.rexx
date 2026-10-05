options levelc
call on error name caught
if 1 then say 'if-body'
call off error
call on error name caught
select
  when 1 then say 'when-body'
  otherwise say 'unexpected'
end
call off error
call on error name caught
do 1
  say 'do-body'
end
call off error
exit
caught:
  say 'trap=' || sigl || '|' || condition('C') || '|' || condition('D')
  return
