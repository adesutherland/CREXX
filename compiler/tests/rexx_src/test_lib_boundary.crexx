/* Standard Library Boundary Tests */
options levelb
import rxfnsb

say "Testing Library Context Isolation..."

stemA = .stem()
stemB = .stem()

call stemA.set("key", "value_A")
call stemB.set("key", "value_B")

say "stemA key =" stemA.get("key")
say "stemB key =" stemB.get("key")

if stemA.get("key") = stemB.get("key") then
  say "FAILED: stemA and stemB are colliding!"

/* Add more keys to verify array boundaries */
call stemA.set("another", "foo")
say "stemB another =" stemB.get("another")

return 0
