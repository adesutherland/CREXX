/* rexx test abs bif */
options levelb
say "Look for DELWORD OK"
if delword('Now is the time',2,2) \= 'Now time' then say 'failed in test 1 '
if delword('Now is the time ',3) \= 'Now is ' then say 'failed in test 2 '
if delword('Now time',5) \= 'Now time' then say 'failed in test 3 '
/* These from Mark Hessling. */
if delword("Med lov skal land bygges", 3) \= "Med lov " then say 'failed in test 4 '
if delword("Med lov skal land bygges", 1) \= "" then say 'failed in test 5 '
if delword("Med lov skal land bygges", 1,1) \= "lov skal land bygges" then say 'failed in test 6 '
if delword("Med lov skal land bygges", 2,3) \= "Med bygges" then say 'failed in test 7 '
if delword("Med lov skal land bygges", 2,10) \= "Med " then say 'failed in test 8 '
if delword("Med lov skal land bygges", 3,2) \= "Med lov bygges" then say 'failed in test 9 '
if delword("Med lov skal land bygges", 3,2) \= "Med lov bygges" then say 'failed in test 10 '
if delword("Med lov skal land bygges", 3,2) \= "Med lov bygges" then say 'failed in test 11 '
if delword("Med lov skal land bygges", 3,0) \= "Med lov skal land bygges" then say 'failed in test 12 '
if delword("Med lov skal land bygges", 10) \= "Med lov skal land bygges" then say 'failed in test 13 '
if delword("Med lov skal land bygges", 9,9) \= "Med lov skal land bygges" then say 'failed in test 14 '
if delword("Med lov skal land bygges", 1,0) \= "Med lov skal land bygges" then say 'failed in test 15 '
if delword(" Med lov skal", 1,0) \= " Med lov skal" then say 'failed in test 16 '
if delword(" Med lov skal ", 4) \= " Med lov skal " then say 'failed in test 17 '
if delword("", 1) \= "" then say 'failed in test 18 '

x='CREXX is faster than BREXX'
if delword(x,1) \= ' ' then say 'failed in test 19 '
if delword(x,2) \= 'CREXX ' then say 'failed in test 20 '
if delword(x,3)  \= 'CREXX is ' then say 'failed in test 21 '
if delword(x,4)  \= 'CREXX is faster '  then say 'failed in test 22 '
if delword(x,5) \= 'CREXX is faster than ' then say 'failed in test 23 '
if delword(x,99)  \= 'CREXX is faster than BREXX' then say 'failed in test 24 '

if delword(x,1,1) \= 'is faster than BREXX' then say 'failed in test 25 '
if delword(x,2,1) \= 'CREXX faster than BREXX' then say 'failed in test 26 '
if delword(x,3,1) \= 'CREXX is than BREXX' then say 'failed in test 27 '
if delword(x,4,1) \= 'CREXX is faster BREXX' then say 'failed in test 28 '
if delword(x,5,1) \= 'CREXX is faster than ' then say 'failed in test 29 '
if delword(x,99,1) \= 'CREXX is faster than BREXX' then say 'failed in test 30 '

if delword(x,1,2) \= 'faster than BREXX' then say 'failed in test 31 '
if delword(x,2,3) \= 'CREXX BREXX' then say 'failed in test 32 '
if delword(x,3,4) \= 'CREXX is ' then say 'failed in test 33 '
if delword(x,4,2) \= 'CREXX is faster ' then say 'failed in test 34 '
if delword(x,5,3) \= 'CREXX is faster than ' then say 'failed in test 35 '
if delword(x,99,4) \= 'CREXX is faster than BREXX' then say 'failed in test 36 '

/* say "delword(x,1) '"delword(x,1)"'" */
/* say "delword(x,2) '"delword(x,2)"'" */
/* say "delword(x,3) '"delword(x,3)"'" */
/* say "delword(x,4) '"delword(x,4)"'" */
/* say "delword(x,5) '"delword(x,5)"'" */
/* say "delword(x,99) '"delword(x,99)"'" */

/* say "delword(x,1,1) '"delword(x,1,1)"'" */
/* say "delword(x,2,1) '"delword(x,2,1)"'" */
/* say "delword(x,3,1) '"delword(x,3,1)"'" */
/* say "delword(x,4,1) '"delword(x,4,1)"'" */
/* say "delword(x,5,1) '"delword(x,5,1)"'" */
/* say "delword(x,99,1) '"delword(x,99,1)"'" */

/* say "delword(x,1,2) '"delword(x,1,2)"'" */
/* say "delword(x,2,3) '"delword(x,2,3)"'" */
/* say "delword(x,3,4) '"delword(x,3,4)"'" */
/* say "delword(x,4,2) '"delword(x,4,2)"'" */
/* say "delword(x,5,3) '"delword(x,5,3)"'" */
/* say "delword(x,99,4) '"delword(x,99,4)"'" */

say "DELWORD OK"
return

/* function prototype */
delword: procedure = .string
arg string1 = .string, wnum = .int, wcount = 0


