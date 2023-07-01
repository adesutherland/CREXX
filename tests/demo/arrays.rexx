#!/usr/local/crexx/rexx.sh
/* Arrays */
options levelb
import rxfnsb /* This imports the standard library - for length() etc. */

a_1_base.1 = "First Line"
a_1_base.2 = "Second Line"
a_1_base.3 = "Third Line"

say "Step 1"
call print_array a_1_base
say

a_1_base[4] = "Forth Line"

say "Step 2"
call print_array a_1_base
say

/* Implicit gives mode power */
complex = .string[-5 to -1]

complex[-5] = "Minus 5"
complex[-4] = "Minus 4"
complex[-3] = "Minus 3"
complex[-2] = "Minus 2"
complex[-1] = "Minus 1"

do i = -5 to -1
    say "Element" i "is" complex.i
end

print_array: procedure
    arg array = .string[]

    do i = 1 to array.0
        say "Element" i "is" array.i
    end
    return