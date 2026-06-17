/* Focused local-inlining benchmark: small multi-argument helper with modest control flow */
options levelb

iterations = 10000000

checksum = 0
carry = 17

do i = 1 to iterations
  carry = mix_step(i, carry, 13)
  checksum = checksum + carry
  if checksum > 1000000 then
    checksum = checksum - 999983
end

say "benchmark=inlining_complex"
say "iterations="iterations
say "carry="carry
say "checksum="checksum

return 0

mix_step: procedure = .int
  arg value = .int, carry = .int, tweak = .int
  mixed = carry + tweak
  if value > 5000000 then
    mixed = mixed + tweak + 3
  else
    mixed = mixed + 1
  if mixed > 1000000 then
    mixed = mixed - 999983
  return mixed
