/* rexx test abs bif */
options levelb
import rxfnsb

errors=0
/* These from TRL */
if delstr('abcd',3) \= 'ab' then do
  errors=errors+1 
  say 'DELSTR failed in test 1 '
end
if delstr('abcde',3,2) \= 'abe' then do
  errors=errors+1 
  say 'DELSTR failed in test 2 '
end
if delstr('abcde',6) \= 'abcde' then do
  errors=errors+1 
  say 'DELSTR failed in test 3 '
end
/* These from Mark Hessling. */
if delstr("Med lov skal land bygges", 6) \= "Med l" then do
  errors=errors+1 
  say 'DELSTR failed in test 4 '
end
if delstr("Med lov skal land bygges", 6,10) \= "Med lnd bygges" then do
  errors=errors+1 
  say 'DELSTR failed in test 5 '
end
if delstr("Med lov skal land bygges", 1) \= "" then do
  errors=errors+1 
  say 'DELSTR failed in test 6 '
end
if delstr("Med lov skal", 30) \= "Med lov skal" then do
  errors=errors+1 
  say 'DELSTR failed in test 7 '
end
if delstr("Med lov skal", 8 , 8) \= "Med lov" then do
  errors=errors+1 
  say 'DELSTR failed in test 8 '
end
if delstr("Med lov skal", 12) \= "Med lov ska" then do
  errors=errors+1 
  say 'DELSTR failed in test 9 '
end
if delstr("Med lov skal", 13) \= "Med lov skal" then do
  errors=errors+1 
  say 'DELSTR failed in test 10 '
end
if delstr("Med lov skal", 14) \= "Med lov skal" then do
  errors=errors+1 
  say 'DELSTR failed in test 11 '
end
if delstr("", 30) \= "" then do
  errors=errors+1 
  say 'DELSTR failed in test 12 '
end

x='CREXX is faster than BREXX'
if delstr(x,6,100) \= 'CREXX' then do
  errors=errors+1 
  say 'DELSTR failed in test 13' 
end
if delstr(x,5) \= 'CREX' then do
  errors=errors+1 
  say 'DELSTR failed in test 14 '
end
if delstr(x,1) \= '' then do
  errors=errors+1 
  say 'DELSTR failed in test 15 '
end
if delstr(x,99) \= 'CREXX is faster than BREXX' then do
  errors=errors+1 
  say 'DELSTR failed in test 16 '
end
if delstr(x,0) \= '' then do
  errors=errors+1 
  say 'DELSTR failed in test 17 '
end

/* say "5   " "'"delstr(x,5)"'" */
/* say "1   " "'"delstr(x,1)"'" */
/* say "99  " "'"delstr(x,99)"'" */
/* say "0   " "'"delstr(x,0)"'" */
return errors<>0
