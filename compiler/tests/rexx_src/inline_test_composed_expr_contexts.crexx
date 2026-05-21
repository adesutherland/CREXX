options levelb

main: procedure = .int
  say "Starting composed inline contexts test..."

  say leftValue(3) + rightValue(4)
  say "prefix:" || labelFor(leftValue(2)) || ":" || rightText(5)
  say combine(leftValue(1), rightValue(2))

  calls = 0
  if zero() & bump(calls) then say "bad-and-skip"
  else say "and-skip" calls

  if one() | bump(calls) then say "or-skip" calls
  else say "bad-or-skip"

  if one() & bump(calls) then say "and-hit" calls
  else say "bad-and-hit"

  if zero() | bump(calls) then say "or-hit" calls
  else say "bad-or-hit"

  say "Composed inline contexts test finished."
  return 0

leftValue: procedure = .int
  arg value = .int
  return value * 10

rightValue: procedure = .int
  arg value = .int
  return value + 7

rightText: procedure = .string
  arg value = .int
  return value

labelFor: procedure = .string
  arg value = .int
  return "[" || value || "]"

combine: procedure = .int
  arg left = .int, right = .int
  return left + right

zero: procedure = .int
  return 0

one: procedure = .int
  return 1

bump: procedure = .int
  arg expose count = .int
  count = count + 1
  return 1
