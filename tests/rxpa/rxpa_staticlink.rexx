/* rxpa (plugin architecture) test 1 */

options levelb  /* This is a rexx level b program */

say "RXPA Compiler Static Link Test"

result1 = proc1()
if result1 \= "static proc1 output" then do
    say 'proc1() unexpectedly returned' result1
    return 1
end

result2 = proc2()
if result2 \= "static proc2 output" then do
    say 'proc2() unexpectedly returned' result2
    return 1
end

return 0
