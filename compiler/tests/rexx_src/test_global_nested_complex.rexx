options levelb
namespace test_complex expose x y z

main: procedure
  x = .int; y = .int; z = .int
  x = 1; y = 2; z = 3
  
  say "Initial: x="||x "y="||y "z="||z
  
  do
    /* Shadow x, but let y and z be auto-exposed */
    x = .int
    x = 10
    y = 20 /* Should update global y */
    say "  Sub 1: x="||x "y="||y "z="||z
    
    do
      /* Shadow y, let x (from Sub 1) and z (global) be auto-exposed */
      y = .int
      y = 200
      x = 11  /* Should update x from Sub 1 */
      z = 300 /* Should update global z */
      say "    Sub 2: x="||x "y="||y "z="||z
    end
    
    say "  Back Sub 1: x="||x "y="||y "z="||z
  end
  
  say "Final: x="||x "y="||y "z="||z
  
  /* Validation */
  if x = 1 & y = 20 & z = 300 then say "SUCCESS: Complex nested scoping with globals OK"
  else say "FAILURE: Scoping error"
  
  return
