options levelb
count = 0
do i = 1 to 5
  if i % 2 = 0 then say i || " is even"
  else say i || " is odd"
  count = count + 1
end

j = 3
do while j > 0
  say "j=" || j
  j = j - 1
  count = count + 1
end

k = 1
do until k = 3
  say "k=" || k
  k = k + 1
  count = count + 1
end
say "Total: " || count
return
