#!/usr/local/crexx/rexx.sh
/* Address testbed */
options levelb
namespace test
import rxfnsb

/* Test Parameters */
program = "syntaxerror.rexx"
expected_rc = 2
expected_result.1 = 'Error in syntaxerror.rexx @ 7:1 - #MISSING_END, "do"'
expected_result.2 = '1 error(s) in source file'

/* Run the test */
say "Testing" program
result = .string[]
address cmd "rxc" program error result

/* Check the expected result */
if rc <> expected_rc | ,
   result.0 <> expected_result.0 | ,
   result.1 <> expected_result.1 | ,
   result.2 <> expected_result.2 then do

    /* Report result */
    say "Error in" program "unexpected result"
    say "Result rc =" rc
    say "Result stderr:"
    do i = 1 to result.0
        say ">" result.i
    end
end
else say "Success in" program