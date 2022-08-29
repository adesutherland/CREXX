/* Tests for latest show and tells */
options levelb
namespace scratch
import rxfnsb
# comment
say "Début"

a  = 0
do i = 1 to 10

  a = a + i
  say right(i,5,"0")
end
say "a =" a

return 0
