options levelb

call test_opt 1, , 3
call test_opt 1, 2
call test_opt , 2, 3
return 0

test_opt: procedure
  arg a=10, b=20, c=30
  say "a=" a "b=" b "c=" c
  return
