options levelb
namespace refcomputed expose idx

main: procedure = .int
  values = .int[3]
  idx = 1
  values[1] = 10
  values[2] = 20

  say bump(values[idx + 0 + 1])
  say idx
  say values[2]
  return 0

bump: procedure = .int
  arg expose slot = .int
  idx = idx + 1
  slot = slot + idx
  return slot
