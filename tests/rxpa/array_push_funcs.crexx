/* rxpa (plugin architecture) array push helper test */
/* This runs against the rxpa_dynlink and rxpa_staticlink plugins */

options levelb
import rxpatests

say "RXPA Array Push Tests"

int_array = .int[]
string_array = .string[]

call fill_push_arrays int_array, string_array

if int_array[0] \= 2 | int_array[1] \= 7 | int_array[2] \= 8 then do
    say 'fill_push_arrays() produced unexpected integer values' int_array[0] int_array[1] int_array[2]
    return 1
end

if string_array[0] \= 2 | string_array[1] \= "alpha" | string_array[2] \= "beta" then do
    say 'fill_push_arrays() produced unexpected string values' string_array[0] string_array[1] string_array[2]
    return 1
end

say "OK"

return 0
