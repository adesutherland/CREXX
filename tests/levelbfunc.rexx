/* Level B Function Test */
options levelb
x = 2.0
call test 1,x, x + 3.0
say x
return

test: procedure = .int
  arg a = .int, b .int, c = .float
  result = 0
  result = a + b + test2(b)
  if result = 3 then do
     say "The Result is 3"
  end
  b = 10
  say c
  return result;

test2: procedure = .float
  arg x .float
  say "starting"
  start_time = 0
  end_time = 0
  a = 0
  assembler say "hello"
  assembler do
    itos a
    say a
  end
  assembler time start_time
  do i = 1 to 100000000
    a = a + i
  end
  assembler time end_time

  say "Time taken is" end_time-start_time "seconds"
  return 0