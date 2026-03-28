/* Focused local-inlining benchmark: tiny single-argument helper in a large loop */
options levelb

iterations = 20000000

checksum = 0

do i = 1 to iterations
  checksum = checksum + add_step(i)
  if checksum > 1000000 then
    checksum = checksum - 999983
end

say "benchmark=inlining_simple"
say "iterations="iterations
say "checksum="checksum

return 0

add_step: procedure = .int
  arg value = .int
  return value - value + 7
