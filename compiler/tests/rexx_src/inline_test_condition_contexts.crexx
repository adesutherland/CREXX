options levelb

main: procedure = .int
  say "Starting condition context inline test..."

  if isOne(1) then say "if-yes"
  else say "bad-if-yes"

  if isOne(2) then say "bad-if-no"
  else say "if-no"

  i = 0
  do while belowTwo(i)
    say "while" i
    i = i + 1
  end

  j = 0
  do until atLeastTwo(j)
    say "until" j
    j = j + 1
  end

  say "Condition context inline test finished."
  return 0

isOne: procedure = .int
  arg value = .int
  return value = 1

belowTwo: procedure = .int
  arg value = .int
  return value < 2

atLeastTwo: procedure = .int
  arg value = .int
  return value >= 2
