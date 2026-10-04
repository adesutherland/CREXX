options levelc
i=10
order=''
do i=startvalue() to tovalue() by byvalue() for forvalue()
  say 'TBF' i order
end i
say 'after-TBF' i order
i=10
order=''
do i=startvalue() to tovalue() for forvalue() by byvalue()
  say 'TFB' i order
end i
say 'after-TFB' i order
i=10
order=''
do i=startvalue() by byvalue() to tovalue() for forvalue()
  say 'BTF' i order
end i
say 'after-BTF' i order
i=10
order=''
do i=startvalue() by byvalue() for forvalue() to tovalue()
  say 'BFT' i order
end i
say 'after-BFT' i order
i=10
order=''
do i=startvalue() for forvalue() to tovalue() by byvalue()
  say 'FTB' i order
end i
say 'after-FTB' i order
i=10
order=''
do i=startvalue() for forvalue() by byvalue() to tovalue()
  say 'FBT' i order
end i
say 'after-FBT' i order
exit
startvalue: procedure expose order i
  order=order||'S'
  return 1
tovalue: procedure expose order i
  order=order||'T'
  return 2
byvalue: procedure expose order i
  order=order||'B'
  return 1
forvalue: procedure expose order i
  order=order||'F'
  return 1
