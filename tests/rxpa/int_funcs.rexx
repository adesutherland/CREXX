/* rxpa (plugin architecture) integer test */
/* This runs against the rxpa_dynlink and rxpa_staticlink plugins */

options levelb
import rxpatests

say "RXPA Integer Tests"

# Test Passing and Returning
result = add_integers(1, 2)
if result \= 3 then do
    say 'add_integers() unexpectedly returned' result
    return 1
end

# Test Pass by Reference
result = 0
call add_integers_ref 1, 2, result
if result \= 3 then do
    say 'add_integers_ref() unexpectedly returned' result
    return 1
end

say "OK"

return 0
