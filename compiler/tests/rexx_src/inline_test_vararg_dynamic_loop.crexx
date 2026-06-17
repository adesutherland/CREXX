options levelb

main: procedure = .int
  say localmax(1, 2)
  say localmax(1, 0)
  say localmax(1, 5)
  return 0

localmax: procedure = .float
  arg m = .float, ... = .float
  do i = 1 to arg.0
    if arg.i > m then m = arg.i
  end
return m
