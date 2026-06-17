/* rxpa (plugin architecture) float test */
/* This runs against the rxpa_dynlink and rxpa_staticlink plugins */

options levelb
import rxpatests

say "RXPA Float Tests"

# Test Passing and Returning
result = add_floats(1.0, 2.0)
if result \= 3.0 then do
    say 'add_floats() unexpectedly returned' result
    return 1
end

# Test Pass by Reference
result = 0.0
call add_floats_ref 1.0, 2.0, result
if result \= 3.0 then do
    say 'add_floats_ref() unexpectedly returned' result
    return 1
end

say "OK"

return 0
