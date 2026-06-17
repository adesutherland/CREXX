options levelb

main: procedure = .int
  pair = .int[2]
  pair = forwardPair(3)
  say pair[1]
  say pair[2]
  return 0

forwardPair: procedure = .int[2]
  arg seed = .int
  return buildPair(seed)

buildPair: procedure = .int[2]
  arg seed = .int
  pair = .int[2]
  pair[1] = seed
  pair[2] = seed + 1
  return pair
