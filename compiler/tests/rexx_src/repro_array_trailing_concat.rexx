options levelb

main: procedure
  abc[1] = "abc_1"
  abc[2] = "abc_2"
  say "out <"abc[1]">"
  say "out <"abc[2]">"
  do i = 1 to 2
    say "in <"abc[i]">"
  end
  return
