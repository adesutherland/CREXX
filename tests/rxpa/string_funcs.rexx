/* rxpa (plugin architecture) string test */
/* This runs against the rxpa_dynlink and rxpa_staticlink plugins */

options levelb
import rxpatests

say "RXPA String Tests"

# Test Passing and Returning
result = string_concat("Hello, ", "World!")
if result \= "Hello, World!" then do
    say 'string_concat() unexpectedly returned' result
    return 1
end

# Test Pass by Reference
result = ""
call string_concat_ref "Hello, ", "World!", result
if result \= "Hello, World!" then do
    say 'string_concat_ref() unexpectedly returned' result
    return 1
end

say "OK"

return 0
