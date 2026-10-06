options levelc
arg mode
rc = 77
select
  when mode = 'MAIN' then exit 7
  when mode = 'BARE' then exit
  when mode = 'LOCAL' then do
    call recurse 2
    say 'resumed-local'
  end
  when mode = 'PRIVATE' then do
    call private
    say 'resumed-private'
  end
  when mode = 'FUNCTION' then say 'resumed-function=' || stop() || later()
  when mode = 'ACTUAL' then do
    call receiver stop(), later()
    say 'resumed-actual'
  end
  when mode = 'ASSIGN' then do
    assigned = stop()
    say 'resumed-assignment'
  end
  when mode = 'RETURN_EXPR' then say 'resumed-return=' || returning()
  when mode = 'LOOP' then do index = 1 to 3
    if index = 2 then exit 13
    say 'loop=' || index
  end
  when mode = 'EMPTY' then exit ''
  when mode = 'TEXT' then exit 'snow=雪'
  otherwise exit 99
end
say 'resumed-main'
exit 98
recurse:
arg depth
if depth > 0 then call recurse depth - 1
if depth = 0 then do
  say 'deepest;rc=' || rc
  exit 8
end
say 'resumed-recurse'
return
private:
procedure expose rc
rc = 88
if 1 then exit 9
say 'resumed-private-body'
return
stop:
exit 10
later:
say 'evaluated-later'
return 11
receiver:
say 'called-receiver'
return
returning:
return stop()
