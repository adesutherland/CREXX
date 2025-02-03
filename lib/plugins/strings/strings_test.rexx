/* STRING Plugin Test */
options levelb
import strings
import rxfnsb

/* strings_test.rexx */

/* Test for xwords function */
say "Test Case 1: xwords"
say "Function: xwords('Hello world! This is a test.', ' ')"
count1 = xwords("Hello world! This is a test.", " ")
say "Expected: 6, Got:" count1
if count1 \= 6 then say "*** xwords does not match"

say "Test Case 2: xwords"
say "Function: xwords('', ' ')"
count2 = xwords("", " ")
say "Expected: 0, Got:" count2
if count2 \= 0 then say "*** xwords does not match"

say "Test Case 3: xwords"
say "Function: xwords('Hello,,world!', ',')"
count3 = xwords("Hello,,world!", ",")
say "Expected: 2, Got:" count3
if count3 \= 2 then say "*** xwords does not match"

/* Test for xword function */
say "Test Case 4: xword"
say "Function: xword('Hello world! This is a test.', 2, ' ')"
xword1 = xword("Hello world! This is a test.", 2, " ")
say "Expected: world!, Got:" xword1
if xword1 \= "world!" then say "*** xword does not match"

say "Test Case 5: xword"
say "Function: xword('Hello world!', 5, ' ')"
xword2 = xword("Hello world!", 5, " ")
say "Expected: , Got:" xword2
if xword2 \= "" then say "*** xword does not match"

say "Test Case 6: xword"
say "Function: xword('Hello world!', 1, ' ')"
xword3 = xword("Hello world!", 1, " ")
say "Expected: Hello, Got:" xword3
if xword3 \= "Hello" then say "*** xword does not match"

/* Test for xlastword function */
say "Test Case 7: xlastword"
say "Function: xlastword('Hello world! This is a test.', ' ')"
xlastword1 = xlastword("Hello world! This is a test.", " ")
say "Expected: test., Got:" xlastword1
if xlastword1 \= "test." then say "*** xlastword does not match"

say "Test Case 8: xlastword"
say "Function: xlastword('', ' ')"
xlastword2 = xlastword("", " ")
say "Expected: , Got:" xlastword2
if xlastword2 \= "" then say "*** xlastword does not match"

say "Test Case 9: xlastword"
say "Function: xlastword('Hello', ' ')"
xlastword3 = xlastword("Hello", " ")
say "Expected: Hello, Got:" xlastword3
if xlastword3 \= "Hello" then say "*** xlastword does not match"

/* Test for xwordindex function */
say "Test Case 10: xwordindex"
say "Function: xwordindex('Hello world! This is a test.', 3, ' ')"
index1 = xwordindex("Hello world! This is a test.", 3, " ")
say "Expected: 14, Got:" index1
if index1 \= 14 then say "*** xwordindex does not match"

say "Test Case 11: xwordindex"
say "Function: xwordindex('Hello world!', 5, ' ')"
index2 = xwordindex("Hello world!", 5, " ")
say "Expected: 0, Got:" index2
if index2 \= 0 then say "*** xwordindex does not match"

say "Test Case 12: xwordindex"
say "Function: xwordindex('Hello world!', 1, ' ')"
index3 = xwordindex("Hello world!", 1, " ")
say "Expected: 1, Got:" index3
if index3 \= 1 then say "*** xwordindex does not match"

/* Test for xpos function */
say "Test Case 16: xpos"
say "Function: xpos('lo', 'Hello', 0)"
xpos1 = xpos("lo", "Hello", 0)
say "Expected: 4, Got:" xpos1
if xpos1 \= 4 then say "*** xpos does not match"

say "Test Case 17: xpos"
say "Function: xpos('world', 'Hello', 0)"
xpos2 = xpos("world", "Hello", 0)
say "Expected: 0, Got:" xpos2
if xpos2 \= 0 then say "*** xpos does not match"

say "Test Case 18: xpos"
say "Function: xpos('Hello', 'Hello', 0)"
xpos3 = xpos("Hello", "Hello", 0)
say "Expected: 1, Got:" xpos3
if xpos3 \= 1 then say "*** xpos does not match"

/* Test for xsubstr function */
say "Test Case 19: xsubstr"
say "Function: xsubstr('Hello world!', 1, 5)"
sub1 = xsubstr("Hello world!", 1, 5)
say "Expected: Hello, Got:" sub1
if sub1 \= "Hello" then say "*** xsubstr does not match"

say "Test Case 20: xsubstr"
say "Function: xsubstr('Hello world!', 7, -1)"
sub2 = xsubstr("Hello world!", 7, -1)
say "Expected: world!, Got:" sub2
if sub2 \= "world!" then say "*** xsubstr does not match"

say "Test Case 21: xsubstr"
say "Function: xsubstr('Hello', 10, 5)"
sub3 = xsubstr("Hello", 10, 5)
say "Expected: , Got:" sub3
if sub3 \= "" then say "*** xsubstr does not match"

/* Test for xdelstr function */
say "Test Case 22: xdelstr"
say "Function: xdelstr('Hello world!', 3, 2)"
del1 = xdelstr("Hello world!", 3, 2)
say "Expected: Heo world!, Got:" del1
if del1 \= "Heo world!" then say "*** xdelstr does not match"

say "Test Case 23: xdelstr"
say "Function: xdelstr('Hello', 1, 5)"
del2 = xdelstr("Hello", 1, 5)
say "Expected: , Got:" del2
if del2 \= "" then say "*** xdelstr does not match"

say "Test Case 24: xdelstr"
say "Function: xdelstr('Hello', 10, 2)"
del3 = xdelstr("Hello", 10, 2)
say "Expected: Hello, Got:" del3
if del3 \= "Hello" then say "*** xdelstr does not match"

/* Test for xstrip function */
say "Test Case 25: xstrip"
say "Function: xstrip('  Hello  ', 'B', ' ')"
xstrip1 = xstrip("  Hello  ", "B", " ")
say "Expected: Hello, Got: '" xstrip1"'"
if xstrip1 \= "Hello" then say "*** xstrip does not match"

say "Test Case 26: xstrip"
say "Function: xstrip('  Hello  ', 'L', ' ')"
xstrip2 = xstrip("  Hello  ", "L", " ")
say "Expected: Hello  , Got:" xstrip2
if xstrip2 \= "Hello  " then say "*** xstrip does not match"

say "Test Case 27: xstrip"
say "Function: xstrip('  Hello  ', 'T', ' ')"
xstrip3 = xstrip("  Hello  ", "T", " ")
say "Expected:   Hello, Got:" xstrip3
if xstrip3 \= "  Hello" then say "*** xstrip does not match"

/* Test for xabbrev function */
say "Test Case 28: xabbrev"
say "Function: xabbrev('Hel', 'Hello', 3)"
xabbrev1 = xabbrev("Hel", "Hello", 3)
say "Expected: 1, Got:" xabbrev1
if xabbrev1 \= 1 then say "*** xabbrev does not match"

say "Test Case 29: xabbrev"
say "Function: xabbrev('Hel', 'Hello', 4)"
xabbrev2 = xabbrev("Hel", "Hello", 4)
say "Expected: 0, Got:" xabbrev2
if xabbrev2 \= 0 then say "*** xabbrev does not match"

say "Test Case 30: xabbrev"
say "Function: xabbrev('Hello', 'Hello', 5)"
xabbrev3 = xabbrev("Hello", "Hello", 5)
say "Expected: 1, Got:" xabbrev3
if xabbrev3 \= 1 then say "*** xabbrev does not match"

/* Test for WORDPOS function */
say "Test Case 31: WORDPOS"
say "Function: WORDPOS('Hello', 'Hello world! This is a test.')"
wordPos1 = WORDPOS("Hello", "Hello world! This is a test.")
say "Expected: 1, Got:" wordPos1
if wordPos1 \= 1 then say "*** WORDPOS does not match"

say "Test Case 32: WORDPOS"
say "Function: WORDPOS('world', 'Hello world! This is a test.')"
wordPos2 = WORDPOS("world", "Hello world! This is a test.")
say "Expected: 2, Got:" wordPos2
if wordPos2 \= 2 then say "*** WORDPOS does not match"

say "Test Case 33: WORDPOS"
say "Function: WORDPOS('test', 'Hello world! This is a test.')"
wordPos3 = WORDPOS("test", "Hello world! This is a test.")
say "Expected: 6, Got:" wordPos3
if wordPos3 \= 6 then say "*** WORDPOS does not match"

say "Test Case 34: WORDPOS"
say "Function: WORDPOS('notfound', 'Hello world! This is a test.')"
wordPos4 = WORDPOS("notfound", "Hello world! This is a test.")
say "Expected: 0, Got:" wordPos4
if wordPos4 \= 0 then say "*** WORDPOS does not match"

/* Test for xsubword function */
say "Test Case 39: xsubword"
say "Function: xsubword('Hello world! This is a test.', 3, 2)"
xsubword1 = xsubword("Hello world! This is a test.", 3, 2,"")
say "Expected: This is, Got:" xsubword1
if xsubword1 \= "This is" then say "*** xsubword does not match"

say "Test Case 40: xsubword"
say "Function: xsubword('Hello world!', 2, 1,"")"
xsubword2 = xsubword("Hello world!", 2, 1,"")
say "Expected: world!, Got:" xsubword2
if xsubword2 \= "world!" then say "*** xsubword does not match"

say "Test Case 41: xsubword"
say "Function: xsubword('Hello world!', 1, 2)"
xsubword3 = xsubword("Hello world!", 1, 2,"")
say "Expected: Hello world!, Got:" xsubword3
if xsubword3 \= "Hello world!" then say "*** xsubword does not match"

say "Test Case 42: xsubword"
say "Function: xsubword('Hello world!', 3, 1)"
xsubword4 = xsubword("Hello world!", 3, 1,"")
say "Expected: , Got:" xsubword4
if xsubword4 \= "" then say "*** xsubword does not match"

say "Test Case 43: xsubword"
say "Function: xsubword('Hello world!', 0, 1)"
xsubword5 = xsubword("Hello world!", 0, 1,"")
say "Expected: , Got:" xsubword5
if xsubword5 \= "" then say "*** xsubword does not match"

/* Test for xwordlen function */
say "Test Case 44: xwordlen"
say "Function: xwordlen('Hello world! This is a test.', 3)"
xwordlen1 = xwordlen("Hello world! This is a test.", 3,"")
say "Expected: 4, Got:" xwordlen1
if xwordlen1 \= 4 then say "*** xwordlen does not match"

say "Test Case 45: xwordlen"
say "Function: xwordlen('Hello world!', 1)"
xwordlen2 = xwordlen("Hello world!", 1,"")
say "Expected: 5, Got:" xwordlen2
if xwordlen2 \= 5 then say "*** xwordlen does not match"

say "Test Case 46: xwordlen"
say "Function: xwordlen('Hello world!', 2)"
xwordlen3 = xwordlen("Hello world!", 2,"")
say "Expected: 6, Got:" xwordlen3
if xwordlen3 \= 6 then say "*** xwordlen does not match"

say "Test Case 47: xwordlen"
say "Function: xwordlen('Hello world!', 3)"
xwordlen4 = xwordlen("Hello world!", 3,"")
say "Expected: 0, Got:" xwordlen4
if xwordlen4 \= 0 then say "*** xwordlen does not match"

say "Test Case 48: xwordlen"
say "Function: xwordlen('', 1)"
xwordlen5 = xwordlen("", 1,"")
say "Expected: 0, Got:" xwordlen5
if xwordlen5 \= 0 then say "*** xwordlen does not match"

/*
string= "Hello, world! This is a test."
say xxxxpos("Hello",string,1)
say xxxxpos("Hello",string,2)
say "xxxwords "xxxxwords(string,"")
say "xxxwordloc "xxxxwordloc(string,10,"")
say "xxxwordloc "xxxxwordloc(string,14,"")
say "last xxxword '"xxxxlastword(string,' ')"'"
do j=1 to 7
   say "xxxwordINDEX "j xxxxwordindex(string,j,' ')
end

do j=1 to 12
   say j "'"xxxxword(string,j,' ')"'"
end
say time('l')
do i=1 to 1000000              ## 0.307219
   wrd=xxxxword(string,6,' ')
end
say time('l')
say "xxxword 7 '"wrd"'"
say time('l')
do i=1 to 1000000             ## 1,192109
   wrd=xxxword(string,6)
end
say time('l')
say "xxxword 7O '"wrd"'"
exit
*/