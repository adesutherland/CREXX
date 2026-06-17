options levelc
signal missing
say afterMissing
signal good
good:
say afterGood
do i = 1 to 1
inner:
  say inside
end
signal inner
call inner
call date
call external_proc
routine:
procedure
say afterRoutine
say beforeBadProcedure
procedure
say afterBadProcedure
