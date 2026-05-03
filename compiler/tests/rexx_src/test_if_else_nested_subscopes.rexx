options levelb
namespace test_if_nested expose x y z

main: procedure
  x = .int; y = .int; z = .int
  x = 1; y = 2; z = 3
  say "Initial: x="||x "y="||y "z="||z
  
  if 1 then do
    x = .int
    x = 10
    y = 20 /* auto-exposure */
    say "  Inside IF DO: x="||x "y="||y "z="||z
    
    do
      y = .int
      y = 200
      z = 300 /* auto-exposure */
      say "    Inside Nested DO in IF: x="||x "y="||y "z="||z
    end
    
    say "  Back IF DO: x="||x "y="||y "z="||z
  end
  
  say "Final: x="||x "y="||y "z="||z
  
  /* Verification logic for IF block */
  errors = 0
  if x \= 1 then errors = errors + 1
  if y \= 20 then errors = errors + 1
  if z \= 300 then errors = errors + 1
  
  if errors = 0 then say "SUCCESS: IF subscopes work correctly"
  else say "FAILURE: IF subscopes failed, x="||x "y="||y "z="||z
  
  return
