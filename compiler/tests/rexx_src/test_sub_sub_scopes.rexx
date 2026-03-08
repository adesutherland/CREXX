options levelb
namespace test_sub_sub_scopes expose x

main: procedure
  x = .int
  x = 1
  say "Initial global x (should be 1):" x
  
  do
    x = .int
    x = 10
    say "Subscope 1 x (should be 10):" x
    
    do
      x = .int
      x = 100
      say "Sub-sub-scope 2 x (should be 100):" x
      
      do
        x = .int
        x = 1000
        say "Sub-sub-sub-scope 3 x (should be 1000):" x
      end
      
      say "Back in sub-sub-scope 2 x (should be 100):" x
    end
    
    say "Back in subscope 1 x (should be 10):" x
  end
  
  say "Final global x (should be 1):" x
  
  if x = 1 then say "SUCCESS: Scoping works correctly"
  else say "FAILURE: Scoping failed, x =" x
  
  return
