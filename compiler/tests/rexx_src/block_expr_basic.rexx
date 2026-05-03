options levelb

total = 10 + do
  a = 5
  if a > 0 then leave with a * 2
  leave with 0
end
say total

nested = do
  leave with 100 + do
    x = 3
    if x = 3 then leave with 7
    leave with 0
  end
end
say nested

say 1 + do
  y = 1
  leave with y
end
