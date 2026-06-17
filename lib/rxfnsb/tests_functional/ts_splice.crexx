/* rexx test abs bif */
options levelb
import rxfnsb
say "=== SPLICE() tests (no padding) ==="
say
errors=0
errors=errors+test_Splice("1) basic replace (middle)"        , "abcdef", "X"   , 3 , 2 , "abXef")
errors=errors+test_Splice("2) grow (needle longer)"         , "abcdef", "XYZ" , 3 , 2 , "abXYZef")
errors=errors+test_Splice("3) shrink (needle shorter)"      , "abcdef", "X"   , 3 , 4 , "abX")
errors=errors+test_Splice("4) insert (len=0)"               , "abcdef", "-"   , 4 , 0 , "abc-def")
errors=errors+test_Splice("5) replace from start"           , "abcdef", "XY"  , 1 , 3 , "XYdef")
errors=errors+test_Splice("6) replace whole string"         , "abcdef", "XYZ" , 1 , 6 , "XYZ")
errors=errors+test_Splice("7) at beyond end -> append"      , "abcdef", "END" , 10, 2 , "abcdefEND")
errors=errors+test_Splice("8) delete tail (needle='')"      , "abcdef", ""    , 4 , 10, "abc")
errors=errors+test_Splice("9) empty haystack insert"        , ""      , "X"   , 1 , 0 , "X")
errors=errors+test_Splice("10) empty haystack replace"      , ""      , "X"   , 1 , 5 , "X")
errors=errors+test_Splice("11) at<1 -> no-op"               , "abcdef", "X"   , 0 , 2 , "abcdef")
errors=errors+test_Splice("12) len<0 -> treated as 0"       , "abcdef", "X"   , 3 , -5, "abXcdef")
errors=errors+test_Splice("13) replace last char"           , "abcdef", "Z"   , 6 , 1 , "abcdeZ")
errors=errors+test_Splice("14) append explicitly (at=hlen+1)", "abcdef","!"   , 7 , 0 , "abcdef!")
errors=errors+test_Splice("15) needle='', len=0 -> no-op"   , "abcdef", ""    , 3 , 0 , "abcdef")

return errors<>0

test_Splice: procedure=.int
  arg label="", haystack="", needle="", at=1, len=1, expect=""
  got = splice(needle,haystack, at, len)

  if got = expect then do
    say "OK  - "label
    say "      needle='"needle"' at="at" len="len
    say "      haystack='"haystack"'"
    if at>0 then say "      remove  ='"substr(haystack,at,len)"'"
    say "      expect  ='"expect"'"
    say "      got     ='"got"'"
    return 0
  end
  else do
    say "      needle='"needle"' at="at" len="len
    say "      haystack='"haystack"'"
     if at>0 then say "      remove  ='"substr(haystack,at,len)"'"
    say "      expect  ='"expect"'"
    say "      got     ='"got"'"
    return 1
  end
return 1