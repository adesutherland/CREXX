/* Procedure-level expose should not emit duplicate symbol warnings. */
options levelb

main: procedure
  call proc1
  call proc2
  say "but var in main is" var "(a taken constant)"
  return

proc1: procedure = .void expose var
  var = "Hello World"
  return

proc2: procedure = .void expose var
  say "var is" var
  return
