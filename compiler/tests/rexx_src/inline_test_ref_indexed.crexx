options levelb
namespace refidx expose values idx

say "Starting indexed ref inline test..."
values = .int[3]
idx = 1
values[1] = 10
values[2] = 20

say refBump(values[idx])
say values[1]
say values[2]
say "Indexed ref inline test finished."
return 0

refBump: procedure = .int
  arg expose value = .int
  idx = idx + 1
  value = value + 1
  return value
