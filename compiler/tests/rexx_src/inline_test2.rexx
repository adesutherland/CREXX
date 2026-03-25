options levelb

main: procedure = .int
  say "Testing ineligible inlining scenarios..."
  
  /* 1. Large procedure (exceeds node count) */
  say "LargeProc result:" largeProc(1, 2, 3, 4, 5)
  
  /* 2. Procedure with nested call */
  say "NestedCallProc result:" nestedCallProc(10)
  
  /* 3. Method (in a class) */
  o = .MyClass()
  say "Method result:" o.myMethod(20)
  
  /* 4. Procedure with multiple returns (already excluded, but good to keep) */
  say "MultiReturn result:" multiReturn(1)
  
  return 0

/* Should be too large if limit is 25 */
largeProc: procedure = .int
  arg a=.int, b=.int, c=.int, d=.int, e=.int
  x = a + b
  x = x + c
  x = x + d
  x = x + e
  x = x * 2
  x = x / 2
  x = x + 1
  x = x - 1
  x = x * 3
  x = x / 3
  return x

/* Should be excluded because it calls another procedure */
nestedCallProc: procedure = .int
  arg val = .int
  return helper(val)

helper: procedure = .int
  arg v = .int
  return v + 1

/* Methods should be excluded */
MyClass: class
  *: factory
    return

  myMethod: method = .int
    arg v = .int
    return v * 2

/* Already excluded by current logic, but here for completeness */
multiReturn: procedure = .int
  arg v = .int
  if v > 0 then return 1
  return 0
