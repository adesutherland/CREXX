options levelb
import regex
import rxfnsb


levenshtein_cases.1 = "kitten sitting"
levenshtein_cases.2 = "hello hallo"
levenshtein_cases.3 = "test test"
levenshtein_cases.4 = "book cook"
levenshtein_cases.5 = "algorithm logarithm"
levenshtein_cases.6 = "REXX COBOL"
levenshtein_cases.7 = "distance instance"
levenshtein_cases.8 = "- test"           ## - means unset
levenshtein_cases.9 = "test "

do i=1 to 9
    str1=word(levenshtein_cases.i,1)
    if str1="-" then str1=''
    str2=word(levenshtein_cases.i,2)
    distance = levenshtein(str1, str2)
    if distance >= 0 then say left("Levenshtein Distance between '"str1"' and '"str2"' is:",62)distance
    else say ay left("Levenshtein Error calculating distance:",61)distance
    distance = hamming(str1, str2,0)
    if distance >= 0 then say left("Hamming Distance between '"str1"' and '"str2"' is:",62)distance
    else say left("Hamming Error calculating distance:",61)distance
end


/* regex_test.rexx - Test the regex plugin functionality */

/* Define flags */
RX_BASIC    = 0      /* Basic Regular Expressions (BRE) */
RX_EXTENDED = 1      /* Extended Regular Expressions (ERE) */
RX_ICASE    = 2      /* Case insensitive matching */
RX_NEWLINE  = 4      /* Honor newline as special character */
RX_NOSUB    = 8      /* Only report success/failure */

/* Test 1: Basic alphanumeric matching */
say 'Test 1: Basic alphanumeric matching'
/* Expected output:
Match:   Test123
Match:   abc123
Match:   123ABC
*/
pattern = '^[A-Za-z0-9]+$'
test_cases = 'Test123 abc123 123ABC'
handle = rxcompile(pattern, RX_EXTENDED)
do i = 1 to words(test_cases)
    str = word(test_cases, i)
    if rxmatch(handle, str, 0) then
        say 'Match:   ' str
    else
        say 'No match:' str
end
call rxfree handle
/* Test 2: Email validation */
say ' Test 2: Email validation'
/* Expected output:
Testing pattern: ^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$
Test string: test@example.com
Result:     1

Test string: user.name@domain.co.uk
Result:     1

Test string: invalid@email
Result:     0

Test string: bad.email@
Result:     0
*/
pattern = '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$'
emails = 'test@example.com user.name@domain.co.uk invalid@email bad.email@'

wrds=words(emails)
do i = 1 to wrds
    email = word(emails, i)
    say email
    call test_pattern pattern, email, RX_EXTENDED
end

/* Test 3: Phone number formats */
say 'Test 3: Phone number formats'
/* Expected output:
Testing pattern: ^\+?[1-9][0-9]{1,14}$
Test string: +1234567890
Result:     1

Test string: 12345
Result:     1

Test string: +invalid
Result:     0
*/
pattern = '^\+?[1-9][0-9]{1,14}$'
numbers = '+1234567890 12345 +invalid'
do i = 1 to words(numbers)
    number = word(numbers, i)
    call test_pattern pattern, number, RX_EXTENDED
end

/* Test 4: Case insensitive matching */
say 'Test 4: Case insensitive matching'
/* Expected output:
Testing pattern: hello world
Test string: HELLO WORLD
Result:     1

Test string: Hello World
Result:     1

Test string: hello world
Result:     1

Test string: HeLLo WoRLD
Result:     1
*/
pattern = 'hello world'
tests = 'HELLO WORLD Hello World hello world HeLLo WoRLD'
do i = 1 to words(tests) by 2
    test = subword(tests, i)
    call test_pattern pattern, test, RX_EXTENDED + RX_ICASE
end

/* Test 5: Date format validation */
say 'Test 5: Date format validation'
/* Expected output:
Testing pattern: ^(19|20)\d\d[- /.](0[1-9]|1[012])[- /.](0[1-9]|[12][0-9]|3[01])$
Test string: 2023-12-31
Result:     1

Test string: 1999/12/31
Result:     1

Test string: 2023-13-45
Result:     0

Test string: 2023-12-32
Result:     0
*/
pattern = '^(19|20)\d\d[- /.](0[1-9]|1[012])[- /.](0[1-9]|[12][0-9]|3[01])$'
dates = '2023-12-31 1999/12/31 2023-13-45 2023-12-32'
do i = 1 to words(dates)
    date = word(dates, i)
    call test_pattern pattern, date, RX_EXTENDED
end

/* Test 6: URL validation */
say 'Test 6: URL validation'
/* Expected output:
Testing pattern: ^(https?:\/\/)?([\da-z\.-]+)\.([a-z\.]{2,6})([\/\w \.-]*)*\/?$
Test string: https://example.com
Result:     1

Test string: http://sub.domain.co.uk
Result:     1

Test string: invalid.url
Result:     0
*/
pattern = '^(https?:\/\/)?([\da-z\.-]+)\.([a-z\.]{2,6})([\/\w \.-]*)*\/?$'
urls = 'https://example.com http://sub.domain.co.uk invalid.url'
do i = 1 to words(urls)
    url = word(urls, i)
    call test_pattern pattern, url, RX_EXTENDED
end

/* Test 7: Password strength */
say 'Test 7: Password strength (8+ chars, upper, lower, number)'
/* -------------------------------------------------------------------------------------------------
 * pattern="^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)[a-zA-Z\d]{8,}$"
 * The above pattern contains lookahead assertions (`(?=...)`) which are not supported in POSIX Basic Regular Expressions (BRE) or Extended Regular Expressions (ERE).
 * -------------------------------------------------------------------------------------------------
*/
/* Password validation pattern without lookahead */

pattern = '^[a-zA-Z0-9]*[a-z][a-zA-Z0-9]*[A-Z][a-zA-Z0-9]*[0-9][a-zA-Z0-9]*$|'||,
         '^[a-zA-Z0-9]*[a-z][a-zA-Z0-9]*[0-9][a-zA-Z0-9]*[A-Z][a-zA-Z0-9]*$|'||,
         '^[a-zA-Z0-9]*[A-Z][a-zA-Z0-9]*[a-z][a-zA-Z0-9]*[0-9][a-zA-Z0-9]*$|'||,
         '^[a-zA-Z0-9]*[A-Z][a-zA-Z0-9]*[0-9][a-zA-Z0-9]*[a-z][a-zA-Z0-9]*$|'||,
         '^[a-zA-Z0-9]*[0-9][a-zA-Z0-9]*[a-z][a-zA-Z0-9]*[A-Z][a-zA-Z0-9]*$|'||,
         '^[a-zA-Z0-9]*[0-9][a-zA-Z0-9]*[A-Z][a-zA-Z0-9]*[a-z][a-zA-Z0-9]*$'

passwords = 'Password123 weakpass NoNumbers123 short'
do i = 1 to words(passwords)
    pass = word(passwords, i)
    call test_pattern pattern, pass, RX_EXTENDED
end


/* Test 8: Error handling */
say 'Test 8: Error handling'
/* Expected output:
Expected error for (: Unmatched ( or \(
Expected error for [a-z: Unmatched [ or [^
Expected error for (?=.*): Invalid pattern
*/
invalid_patterns = '(' '[a-z' '(?=.*)'
do i = 1 to words(invalid_patterns)
    pat = word(invalid_patterns, i)
    handle = rxcompile(pat, RX_EXTENDED)
    if handle < 0 then
        say 'Expected error for' pat':' rxerror(handle)
end

exit 0


/* Helper function to test patterns */
test_pattern: procedure=.string
    arg pattern = .string, test_string=.string, flags=.int
    say copies('-',72)
    say 'Testing pattern:' pattern
    say 'Test string:' test_string
    say 'Flags:      ' flags

    handle = rxcompile(pattern, flags)
    if handle < 0 then do
        say 'Error compiling pattern:' rxerror(handle)
        return 0
    end
    result = rxmatch(handle, test_string, 0)
    if result>0 then say 'Result:     ' result'   match'
    else say 'Result:     ' result'   no match'
    say copies('-',72)
    call rxfree handle
return result

