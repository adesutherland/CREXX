options levelb
import rxfnsb
namespace regex_test expose  tests fails
/* ------------------------------------------------------------------
   RxLite: Minimal regex engine in pure REXX (no external libs)
Features: ., ^, $, *, +, ?, [abc], [a-z], [^...], \d \D \w \W \s \S
No groups or alternation yet.
------------------------------------------------------------------- */
/* Public: REGEXMATCH(text, pattern)
Returns 1 if pattern matches anywhere in text, else 0.
*/
/* ##cflags def nset  3buf  parse */
/* ------------------------------------------------------------------
   RxLite Test Suite (drop this after your RxLite implementation)
   Assumes: regexmatch, regexfind, REGEXFINDALL, REGEXSPLIT,
            REGEXREPLACE, REGEXREPLACE_LIMIT exist
------------------------------------------------------------------- */
/* ##global int tests=0 fails=0 GL*/
tests=.INT; tests=0
fails=.INT; fails=0
/* ---------- TESTS ---------- */
/* 1) regexmatch basics */
call TEST_TRUE 'regexmatch: abc123 ^\w+\d+$', regexmatch("abc123", "^\w+\d+$")
details=.int[]
call regexdetails details
call TEST_EQ 'len abc123', details.2, 6
say "**** Test2"
call TEST_TRUE 'regexmatch: file.TXT .*\.[A-Za-z]+', regexmatch("file.TXT",".*\.[A-Za-z]+")
call regexdetails details
call TEST_EQ 'len file.TXT',details.2, 8    ## length is in 2. stem entry
say "**** Test 3"
call TEST_TRUE 'regexmatch: color colou?r', regexmatch("color","colou?r")
call TEST_TRUE 'regexmatch: colou?r vs colo+r NO', \regexmatch("colou?r", "colo+r")
/* 2) regexfind / FINDALL tokens */
s = "ID=7; next=42; end."
/* ##array string tx AR*/
tx=.string[];
n = REGEXFINDALL(s, "\d+",tx )
call TEST_EQ 'FINDALL count digits', n, 2
call TEST_EQ 'FINDALL tx.1', tx.1, "7"
call TEST_EQ 'FINDALL tx.2', tx.2, "42"
/* 3) REGEXSPLIT (gaps) */
/* ##array string parts AR*/
parts=.string[];
n = REGEXSPLIT("a,,c,", ",+", parts)
call TEST_EQ 'SPLIT count a,,c,', n, 3
call TEST_EQ 'SPLIT 1 a,,c,', parts.1, "a"
call TEST_EQ 'SPLIT 2 a,,c,', parts.2, "c"
call TEST_EQ 'SPLIT 3 a,,c, (tail empty)', parts.3, ""
/* 4) REPLACE simple */
call TEST_EQ 'REPLACE digits -> #', REGEXREPLACE("ID=7; next=42", "\d+", "#"), "ID=#; next=#"
call TEST_EQ 'REPLACE commas -> |', REGEXREPLACE("a,,c,", ",+", "|"), "a|c|"
call TEST_EQ 'REPLACE colou?r -> X', REGEXREPLACE("color colour","colou?r","X"), "X X"
/* 5) Debug: single class match */
call TEST_TRUE 'regexmatch [0-9] in 1,2,3,4', regexmatch("1,2,3,4", "[0-9]")
rxc=regexdetails(details) ; call TEST_EQ 'substr [0-9]', substr("1,2,3,4", details.1, details.2), "1"
/* 6) REPLACE_LIMIT */
call TEST_EQ 'REPLACE_LIMIT 2 digits', REGEXREPLACE_LIMIT("1,2,3,4","[0-9]","X",2), "X,X,3,4"
call TEST_EQ 'REPLACE_LIMIT all \d', REGEXREPLACE_LIMIT("1,2,3,4","\d","X",0), "X,X,X,X"
call TEST_EQ 'REPLACE_LIMIT ^ once', REGEXREPLACE_LIMIT("abc","^","<>",1), "<>abc"
call TEST_EQ 'REPLACE_LIMIT no digits', REGEXREPLACE_LIMIT("no digits","\d+","#",3), "no digits"
/* 7) Case-insensitive */
call TEST_TRUE 'regexmatch (?i).*\.txt$', regexmatch("File.Txt","(?i).*\.[a-z]+$")
rxc=regexdetails(details)  ; call TEST_EQ '(?i) matched whole file', substr("File.Txt", details.1, details.2), "File.Txt"
/* 8) Greedy vs Lazy replacement */
call TEST_EQ 'greedy X.*X -> #',  REGEXREPLACE("X1X2X3X", "X.*X",  "#"), "#"
call TEST_EQ 'lazy   X.*?X -> ####', REGEXREPLACE("X1X2X3X","X.*?X","#"), "#2#"
/* 9) Alternation (top-level) */
call TEST_TRUE 'alt cat|dog on dog', regexmatch("dog","cat|dog")
call TEST_TRUE 'alt ^a|z$ on abc',  regexmatch("abc","^a|z$")
call TEST_TRUE 'alt ^a|z$ on xyz',  regexmatch("xyz","^a|z$")
call TEST_TRUE 'alt ^a|z$ on bcd (NO)', \regexmatch("bcd","^a|z$")
call TEST_EQ 'REPLACE alt red|green', REGEXREPLACE("red blue green","red|green","X"), "X blue X"
/* regexfind with alternation order */
s = "ant bat cat"
/* ##array string tx2 AR*/
tx2=.string[];
n = REGEXFINDALL(s, "cat|ant", tx2)
call TEST_EQ 'alt order ant then cat count', n, 2
call TEST_EQ 'alt order first', tx2.1, "ant"
call TEST_EQ 'alt order second', tx2.2, "cat"
/* 10) Class fast path & finalize */
call TEST_TRUE 'class fast [A-Z] on A', regexmatch("A","[A-Z]")
rxc=regexdetails(details); call TEST_EQ 'class fast len', details.2, 1
call TEST_EQ 'class fast sub', substr("A", details.1, details.2), "A"
/* 11) Literal pipe + pipe in class */
call TEST_TRUE 'escaped pipe a\|b', regexmatch("a|b","a\|b")
call TEST_TRUE 'pipe in class [|]', regexmatch("|","[|]")
/* 12) Negated class */
call TEST_TRUE 'negated ^[0-9]+$ on abc', regexmatch("abc","^[^0-9]+$")
call TEST_TRUE 'negated ^[0-9]+$ on a1c (NO)', \regexmatch("a1c","^[^0-9]+$")
/* 13) CI class with digits */
call TEST_TRUE '(?i)^[a-z]+[0-9]+$ on MiXeD123', regexmatch("MiXeD123","(?i)^[a-z]+[0-9]+$")
rxc=regexdetails(details); call TEST_EQ 'MiXeD123 len', details.2, 8
/* 14) Overlapping FINDALL */
/* ##array string t3 AR*/
t3=.string[];
n = REGEXFINDALL("aaaa","aa", t3, "O")
call TEST_EQ 'overlap count', n, 3
call TEST_EQ 'overlap 1', t3.1, "aa"
call TEST_EQ 'overlap 2', t3.2, "aa"
call TEST_EQ 'overlap 3', t3.3, "aa"
/* 15) Zero-length at end: $ replacement */
call TEST_EQ 'REPLACE_LIMIT $ once', REGEXREPLACE_LIMIT("abc","$","<>",1), "abc<>"
/* 16) Class range & escaped hyphen */
call TEST_TRUE 'range [a-c] on b', regexmatch("b","[a-c]")
call TEST_TRUE 'escaped hyphen [\-] on -', regexmatch("-","[\-]")
/* 17) REGEXSPLIT options: E (expand) */
/* ##array string t1,t2,a4 AR*/
t1=.string[];
t2=.string[];
a4=.string[];
n = REGEXSPLIT("a,,,b", ",+", t1)
/* default -> "a" "b" */
call EXPECT_STEM 'SPLIT a,,,b basic', "a|b", T1
n = REGEXSPLIT("a,,,b", ",+", t2, "E")
/* expand -> "a" "" "" "b" */
call EXPECT_STEM 'SPLIT a,,,b expand', "a|||b", T2
/* 18) REGEXSPLIT options: TD (trim + drop empties) */
n = REGEXSPLIT("  a , , b  ", "\s*,\s*", a4, "TD")
call EXPECT_STEM 'SPLIT TD trim/drop', "a|b", a4
/* 19) FINDALL limit */
/* ##array string t4 AR*/
t4=.string[];
n = REGEXFINDALL("a1b2c3", "\d", t4, '', 2)
call TEST_EQ 'FINDALL limit count', n, 2
call TEST_EQ 'FINDALL limit 1', t4.1, "1"
call TEST_EQ 'FINDALL limit 2', t4.2, "2"
/* 20) regexfind anchored-alt from offset: should still find the non-anchored alt */
call TEST_TRUE 'regexfind ^foo|bar from 3 finds bar', regexfind("xxbar", "^foo|bar", 3)
rxc=regexdetails(details) ; call TEST_EQ 'regexfind bar start', details.1, 3
/* 21) regexfind class fast-path from offset */
call TEST_TRUE 'regexfind [0-9] from 3 in a1b2', regexfind("a1b2","[0-9]",3)
rxc=regexdetails(details) ; call TEST_EQ 'start digit at 4', details.1, 4
call TEST_EQ 'digit at 4', substr("a1b2", details.1, details.2), "2"
say '************ New Tests ********************'
/* ---- 1) Basic match / no match ---- */
call TEST_TRUE 'match ^\w+\d+$ on "abc123"', REGEXMATCH("abc123", "^\w+\d+$")
call TEST_TRUE 'no match ^\d+$ on "abc"', \REGEXMATCH("abc", "^\d+$")
/* ---- 2) Case-insensitive ---- */
call TEST_TRUE '(?i).*\.txt$ on "File.Txt"', REGEXMATCH("File.Txt", "(?i).*\.[a-z]+$")
/* ---- 3) Character class ---- */
call TEST_TRUE '[0-9] in "1,2,3"', REGEXMATCH("1,2,3", "[0-9]")
/* ---- 4) Alternation (top-level) ---- */
call TEST_TRUE 'alt red|green hits "green"', REGEXMATCH("green", "red|green")
call TEST_TRUE 'alt red|green misses "blue"', \REGEXMATCH("blue", "red|green")
/* ---- 5) Replace (all) ---- */
call TEST_EQ 'replace digits -> X', REGEXREPLACE("1,2,3", "\d", "X"), "X,X,X"
/* ---- 6) Replace with limit ---- */
call TEST_EQ 'replace limit 2', REGEXREPLACE_LIMIT("1,2,3,4", "\d", "X", 2), "X,X,3,4"
/* ---- 7) $0 / $& expansion in replacement ---- */
call TEST_EQ '$0: abc<123>', REGEXREPLACE("abc123", "\d+", "<$0>"), "abc<123>"
call TEST_EQ '$&: abc[123]', REGEXREPLACE("abc123", "\d+", "[$&]"), "abc[123]"
call TEST_EQ '$$ escape',     REGEXREPLACE("a1b", "\d", "$$"),       "a$b"
/* ---- 8) Greedy vs lazy ---- */
call TEST_EQ 'greedy X.*X -> #',  REGEXREPLACE("X1X2X3X", "X.*X",  "#"),  "#"
call TEST_EQ 'lazy   X.*?X -> #2#', REGEXREPLACE("X1X2X3X", "X.*?X", "#"), "#2#"
/* ---- 9) Split (gaps) ---- */
n = REGEXSPLIT("a,,c,", ",+", parts)
call TEST_EQ 'split count a,,c,', n, 3
call TEST_EQ 'split[1]', parts.1, "a"
call TEST_EQ 'split[2]', parts.2, "c"
call TEST_EQ 'split[3] (tail)', parts.3, ""
/* ---- 10) Find-all tokens ---- */
s = "ID=7; next=42; end."
tx.1 = ""                        /* ensure stem exists */
m = REGEXFINDALL(s, "\d+", tx)  /* text-only is enough for smoke */
call TEST_EQ 'findall count', m, 2
call TEST_EQ 'findall[1]', tx.1, "7"
call TEST_EQ 'findall[2]', tx.2, "42"
/* ---- 11) Zero-length anchors in replacement ---- */
call TEST_EQ 'replace "^" -> []abc', REGEXREPLACE("abc", "^", "[]"), "[]abc"
call TEST_EQ 'replace "$" -> abc[]', REGEXREPLACE("abc", "$", "[]"), "abc[]"
/* ---- 12) Negated class whole-line check ---- */
call TEST_TRUE '^[^0-9]+$ on "abc"', REGEXMATCH("abc", "^[^0-9]+$")
call TEST_TRUE '^[^0-9]+$ fails "a1c"', \REGEXMATCH("a1c", "^[^0-9]+$")
/* ---------- summary ---------- */
say tests 'tests,' fails 'failed.'
if fails = 0 then say 'ALL TESTS PASSED'
else say 'SOME TESTS FAILED'

exit
/* ---------- helpers ---------- */
TEST_TRUE: PROCEDURE
  arg label=.string, cond=.int
  tests = tests + 1
  if cond then do
     say '+++ Success 'label': 'label', cond='cond
     return
  end
  fails = fails + 1
  say '--- FAIL 'label': 'label', cond='cond
return
TEST_EQ: PROCEDURE
  arg label=.string, actual=.string, expected=.string
  tests = tests + 1
  if actual = expected then do
     say '+++ Success ' label 'expected="'expected'" got="'actual'"'
     return
  end fails = fails + 1
  say 'FAIL:' label 'expected="'expected'" got="'actual'"'
return
/* verify a stem t. has exactly the pipe-separated items in exp ("a|b||c") */
EXPECT_STEM: PROCEDURE
  arg label=.string,exp=.string, stemname=.string[]
  tests = tests + 1
  /* split exp into e.1..e.n (preserve empties) */
estem=.string[]
estem.1=''
  emax=0
  rest = exp
  do forever
     p = pos('|', rest)
     emax = emax+1
     if p = 0 then do
        estem[emax]=rest
        leave
     end
     estem[emax]=substr(rest, 1, p-1)
     rest = substr(rest, p+1)
  end
  /* compare against target stem */
  ok = 1
do i = 1 to estem.0
v = stemname.i
if v \= estem.i then do
        ok = 0
        leave
     end
  end
  /* also ensure .0 matches if present */
if stemname.0 \= estem.0 then ok = 0
  if ok then return
  fails = fails + 1
  say 'FAIL:' label 'expected: ['exp'] got: ['STEM_JOIN(estem)']'
return
STEM_JOIN: PROCEDURE=.string
  arg stemname=.string[]
  out = ''
do i = 1 to stemname.0
     if i > 1 then out = out || '|'
out = out || stemname.i
  end
return out