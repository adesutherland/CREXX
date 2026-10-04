options levelc
count = 0
do 5
  count = count + 1
  if count = 2 then leave
end
say 'count' count

do forever
  say 'forever'
  leave
end

check = 0
do while 1
  check = check + 1
  leave
end
say 'while' check

check = 0
do until 0
  check = check + 1
  leave
end
say 'until' check

do outer = 1 to 4
  do inner = 1 to 3
    do
      if inner = 2 then leave OuTeR
    end
    say 'inner' outer inner
  end inner
  say 'bad-outer'
end outer
say 'outer-state' outer inner

do same = 1 to 2
  do same = 1 to 3
    leave SAME
  end same
  say 'same-inner' same
  leave same
end same
say 'same-final' same

tail = 'a'
do stem.tail = 1 to 3
  tail = 'b'
  select
    when 1 then leave STEM.TAIL
    otherwise say 'bad-compound'
  end
end stem.tail
say 'compound' stem.a stem.b

do limit = 1 to 4 until 0
  say 'limit-body' limit
  leave limit
end limit
say 'limit-state' limit

say 'local' local()
exit

local:
procedure
do p = 1 to 3
  do q = 1 to 2
    if q = 2 then leave p
  end q
end p
return p
