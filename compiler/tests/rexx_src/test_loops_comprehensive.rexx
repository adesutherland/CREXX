options levelb
namespace test_loops expose g1 g2

main: procedure
  g1 = .int; g2 = .int
  g1 = 1; g2 = 2
  
  /* Scenario 1: Iterative loop with existing global control variable */
  say "S1: Global g1 before:" g1 " (expect 1)"
  do g1 = 1 to 3
     say "  Inside: g1 =" g1
  end
  say "S1: Global g1 after:" g1 " (expect 4 in Classic REXX, or 1 if subscoped)"
  
  /* Scenario 2: Iterative loop with new local control variable */
  say "S2: Local L1 before: L1"
  do L1 = 1 to 2
     say "  Inside: L1 =" L1
  end
  say "S2: Local L1 after:" L1 " (expect L1 if subscoped and didn't exist)"
  
  /* Scenario 3: Iterative loop shadowing global with typed variable */
  say "S3: Global g2 before:" g2 " (expect 2)"
  do i = 1 to 1
     g2 = .int
     g2 = 20
     say "  Inside: g2 =" g2 " (expect 20)"
  end
  say "S3: Global g2 after:" g2 " (expect 2)"
  
  /* Scenario 4: Nested loops and shadowing */
  x = 100
  say "S4: Local x before:" x
  do i = 1 to 2
     x = .int
     x = i
     do j = 1 to 2
        x = x + 10
     end
     say "  Inside outer, after inner: x =" x
  end
  say "S4: Local x after:" x " (expect 100)"
  
  /* Scenario 5: Untyped exposure in loop */
  y = 50
  say "S5: Local y before:" y
  do i = 1 to 1
     y = 60
     say "  Inside: y =" y
  end
  say "S5: Local y after:" y " (expect 60)"
  
  return
