/* Simple Test Rexx Program */
options levelb

say "Starting"

res = "Results"

x = 1
do i = 1 to 5
  x = x + i
  if x // 2 = 0 then say x "is even"
  else say x "is odd"
end

say res
say "i =" i
say "x =" x

say "done!"
return 0