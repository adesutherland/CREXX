options levelc
i=10
calls=0
do i=1 for forvalue() to tovalue() by byvalue()
  say 'bad-first'
end i
say 'first-after' i calls
i=10
calls=0
do i=1 by byvalue() to tovalue() for forvalue()
  say 'bad-second'
end i
say 'second-after' i calls
i=10
calls=0
do i=1 by byvalue() to tovalue() for twocount()
  say 'all' i calls
end i
say 'after-all' i calls
limit=3
do counted=1 for limit
  say 'counted' counted limit
  limit=1
end counted
say 'after-counted' counted limit
decimalcount='2.0'
do decimal=1 for decimalcount
  say 'decimal' decimal
end decimal
say 'after-decimal' decimal
untilcount=2
untilstep=1
do ending=1 for untilcount by untilstep until ending>=2
  say 'until' ending
  untilstep=9
end ending
say 'after-until' ending untilstep
calls=0
condcalls=0
do zero=1 for zerocount() while tick()
  say 'bad-zero'
end zero
say 'after-zero' zero calls condcalls
calls=0
do zeroend=1 for zerocount() until tick()
  say 'bad-zero-end'
end zeroend
say 'after-zero-end' zeroend calls condcalls
count=2
guard=0
do outer=1 for count
  guard=guard+1
  if guard>5 then exit
  do inner=1 to 2
    if inner=1 then iterate OuTeR
  end inner
end outer
say 'after-outer' outer
say local()
exit
forvalue: procedure expose i calls
calls=calls+1
say 'for' i calls
return 0
twocount: procedure expose i calls
calls=calls+1
say 'for-two' i calls
return 2
tovalue: procedure expose i calls
calls=calls+1
say 'to' i calls
return 3
byvalue: procedure expose i calls
calls=calls+1
say 'by' i calls
return 1
zerocount: procedure expose calls
calls=calls+1
return 0
tick: procedure expose condcalls
condcalls=condcalls+1
return 1
local: procedure
localcount=2
do scoped=1 for localcount
  say 'local' scoped
end scoped
return scoped
