options levelb

main: procedure = .int
  say bumpFirst(makePair())
  return 0

bumpFirst: procedure = .int
  arg pair = .int[2]
  pair[1] = pair[1] + 5
  return pair[1]

makePair: procedure = .int[2]
  pair = .int[2]
  a = .int
  b = .int
  c = .int
  d = .int
  e = .int
  f = .int
  a = 1
  b = a + 1
  c = b + 1
  d = c + 1
  e = d + 1
  f = e + 1
  pair[1] = 7
  pair[2] = 9
  return pair
