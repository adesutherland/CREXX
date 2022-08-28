/* rexx test abs bif */
options levelb
import rxfnsb

errors=0

if delword('Now is the time',2,2) \= 'Now time' then do
  errors=errors+1
  say 'DELWORD failed in test 1 '
end
if delword('Now is the time ',3) \= 'Now is ' then do
  errors=errors+1
  say 'DELWORD failed in test 2 '
end
if delword('Now time',5) \= 'Now time' then do
  errors=errors+1
  say 'DELWORD failed in test 3 '
end
  /* These from Mark Hessling. */
if delword("Med lov skal land bygges", 3) \= "Med lov " then do
  errors=errors+1
  say 'DELWORD failed in test 4 '
end
if delword("Med lov skal land bygges", 1) \= "" then do
  errors=errors+1
  say 'DELWORD failed in test 5 '
end
if delword("Med lov skal land bygges", 1,1) \= "lov skal land bygges" then do
  errors=errors+1
  say 'DELWORD failed in test 6 '
end
if delword("Med lov skal land bygges", 2,3) \= "Med bygges" then do
  errors=errors+1
  say 'DELWORD failed in test 7 '
end
if delword("Med lov skal land bygges", 2,10) \= "Med " then do
  errors=errors+1
  say 'DELWORD failed in test 8 '
end
if delword("Med lov skal land bygges", 3,2) \= "Med lov bygges" then do
  errors=errors+1
  say 'DELWORD failed in test 9 '
end
if delword("Med lov skal land bygges", 3,2) \= "Med lov bygges" then do
  errors=errors+1
  say 'DELWORD failed in test 10 '
end
if delword("Med lov skal land bygges", 3,2) \= "Med lov bygges" then do
  errors=errors+1
  say 'DELWORD failed in test 11 '
end
if delword("Med lov skal land bygges", 3,0) \= "Med lov skal land bygges" then do
  errors=errors+1
  say 'DELWORD failed in test 12 '
end
if delword("Med lov skal land bygges", 10) \= "Med lov skal land bygges" then do
  errors=errors+1
  say 'DELWORD failed in test 13 '
end
if delword("Med lov skal land bygges", 9,9) \= "Med lov skal land bygges" then do
  errors=errors+1
  say 'DELWORD failed in test 14 '
end
if delword("Med lov skal land bygges", 1,0) \= "Med lov skal land bygges" then do
  errors=errors+1
  say 'DELWORD failed in test 15 '
end
if delword(" Med lov skal", 1,0) \= " Med lov skal" then do
  errors=errors+1
  say 'DELWORD failed in test 16 '
end
if delword(" Med lov skal ", 4) \= " Med lov skal " then do
  errors=errors+1
  say 'DELWORD failed in test 17 '
end
if delword("", 1) \= "" then do
  errors=errors+1
  say 'DELWORD failed in test 18 '
end

x='CREXX is faster than BREXX'

if delword(x,1) \= '' then do
  errors=errors+1
  say 'DELWORD failed in test 19 '
end
if delword(x,2) \= 'CREXX ' then do
  errors=errors+1
  say 'DELWORD failed in test 20 '
end
if delword(x,3)  \= 'CREXX is ' then do
  errors=errors+1
  say 'DELWORD failed in test 21 '
end
if delword(x,4)  \= 'CREXX is faster '  then do
  errors=errors+1
  say 'DELWORD failed in test 22 '
end
if delword(x,5) \= 'CREXX is faster than ' then do
  errors=errors+1
  say 'DELWORD failed in test 23 '
end
if delword(x,99)  \= 'CREXX is faster than BREXX' then do
  errors=errors+1
  say 'DELWORD failed in test 24 '
end
if delword(x,1,1) \= 'is faster than BREXX' then do
  errors=errors+1
  say 'DELWORD failed in test 25 '
end
if delword(x,2,1) \= 'CREXX faster than BREXX' then do
  errors=errors+1
  say 'DELWORD failed in test 26 '
end
if delword(x,3,1) \= 'CREXX is than BREXX' then do
  errors=errors+1
  say 'DELWORD failed in test 27 '
end
if delword(x,4,1) \= 'CREXX is faster BREXX' then do
  errors=errors+1
  say 'DELWORD failed in test 28 '
end
if delword(x,5,1) \= 'CREXX is faster than ' then do
  errors=errors+1
  say 'DELWORD failed in test 29 '
end
if delword(x,99,1) \= 'CREXX is faster than BREXX' then do
  errors=errors+1
  say 'DELWORD failed in test 30 '
end
if delword(x,1,2) \= 'faster than BREXX' then do
  errors=errors+1
  say 'DELWORD failed in test 31 '
end
if delword(x,2,3) \= 'CREXX BREXX' then do
  errors=errors+1
  say 'DELWORD failed in test 32 '
end
if delword(x,3,4) \= 'CREXX is ' then do
  errors=errors+1
  say 'DELWORD failed in test 33 '
end
if delword(x,4,2) \= 'CREXX is faster ' then do
  errors=errors+1
  say 'DELWORD failed in test 34 '
end
if delword(x,5,3) \= 'CREXX is faster than ' then do
  errors=errors+1
  say 'DELWORD failed in test 35 '
end
if delword(x,99,4) \= 'CREXX is faster than BREXX' then do
  errors=errors+1
  say 'DELWORD failed in test 36 '
end
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
return errors<>0
