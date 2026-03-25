options levelb

/* This test contains procedures that SHOULD NOT be inlined */

main: procedure = .int
  say "Testing non-inlinable procedures..."
  
  /* Procedure with multiple returns */
  r1 = multiReturn(10)
  say "r1:" r1
  
  /* Procedure with reference argument */
  x = 5
  call refArg x
  say "x after refArg:" x
  
  /* Procedure with optional argument */
  call optArg 10
  
  return 0

multiReturn: procedure = .int
  arg val = .int
  if val > 5 then return 1
  return 0

refArg: procedure
  arg v = .int .ref
  v = v + 1
  return

optArg: procedure
  arg a = .int, b = .int .opt
  say a
  return
