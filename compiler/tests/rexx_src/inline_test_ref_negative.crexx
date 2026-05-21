options levelb
namespace refneg expose values idx

say "Starting ref inline negative test..."
values = .int[3]
idx = 1
values[1] = 10
values[2] = 20

if refBump(values[idx]) & 1 then say "branch"
say values[1]
say values[2]
say "Ref inline negative test finished."
return 0

refBump: procedure = .int
  arg expose value = .int
  idx = idx + 1
  value = value + 1
  return value
