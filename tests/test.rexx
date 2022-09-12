/* Tests for latest show and tells */
options levelb
namespace scratch expose main a  p
import rxfnsb

# comment
say "Début"
call xxx

a  = 0
p = 3
do i = 1 to 10

  a = a + i
  say right(i,5,"0")
  say p
end
say "a =" a

return 0

xxx: procedure
x = 5
say "xxx"
return
