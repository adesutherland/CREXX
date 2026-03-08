options levelb
namespace test_if_global expose x y

main: procedure
  x = .int; y = .int
  x = 1; y = 2
  say "Initial: x="||x "y="||y
  
  if 1 then do
    x = .int
    x = 10
    say "  Inside IF DO: x="||x
  end
  say "After IF DO: x =" x " (expect 1)"
  
  if 0 then nop
  else do
    y = .int
    y = 20
    say "  Inside ELSE DO: y="||y
  end
  say "After ELSE DO: y =" y " (expect 2)"
  
  /* Validation */
  errors = 0
  if x \= 1 then do
    say "FAILURE: x was not restored after IF DO"
    errors = errors + 1
  end
  if y \= 2 then do
    say "FAILURE: y was not restored after ELSE DO"
    errors = errors + 1
  end
  
  if errors = 0 then say "SUCCESS: IF/ELSE DO subscoping works"
  else say "FAILURE: Scoping failed"
  
  return
