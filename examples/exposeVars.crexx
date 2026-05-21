/* PROCEDURE EXPOSE demo in CREXX Level B */

options levelb
namespace peterJ expose glob

main: procedure
  glob="Fred"
  call level1
  say "but var in main is" var
return
level1: procedure
  call proc1
  say " var after set in level1 is" var
  call proc2
  say "but var in level1 is" var
return

proc1: procedure = .void expose var
  var = "Hello World"
  say 123 glob
return

proc2: procedure = .void expose var
  say "var is" var
    say 456 glob
return
