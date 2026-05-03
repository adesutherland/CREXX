options levelb
/* Array Intrinsics Test */

say "Testing Array Intrinsics..."

a = .int[10]
len = a[]
say "Length of a[10] (a[]):" len
if len \= 10 then do
    say "FAILED: a[] should be 10"
    return 1
end

b = .string[5]
len_b = b[0] /* Syntax candy for 1-based arrays: 0 means length */
say "Length of b[5] (b[0]):" len_b
if len_b \= 5 then do
    say "FAILED: b[0] should be 5"
    return 1
end

c = .int[3, 4]
len_c = c[1, ] /* Supporting arr[index, void] */
say "Length of c[3, 4] second dimension at index 1 (c[1, ]):" len_c
if len_c \= 4 then do
    say "FAILED: c[1, ] should be 4"
    return 1
end

say "Array Intrinsics Test PASSED"
return 0
