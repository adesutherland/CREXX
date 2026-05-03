options levelb

/* A simple test program to demonstrate pass-by-value procedure called in a loop */

main: procedure = .int
  say "Starting the loop..."
  do i = 1 to 5
    result = multiplyByTwo(i)
    say "Result of multiplyByTwo for" i "is" result
  end
  say "Loop finished."
  return 0

multiplyByTwo: procedure = .int
  arg val = .int
  /* val is passed by value */
  return val * 2
