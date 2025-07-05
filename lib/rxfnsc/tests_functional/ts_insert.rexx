/* rexx test abs bif */
options levelb
import rxfnsb

errors=0

x='CREXX is faster than BREXX'

if insert('much',x,10,10) \= "CREXX is fmuch      aster than BREXX" then do
  errors=errors+1
  say 'INSERT failed in test 1' insert('much',x,10,10)
end

if insert('much ',x,10) \= 'CREXX is fmuch aster than BREXX' then do
  errors=errors+1
  say 'INSERT failed in test 2' insert('much ',x,10)
end

if insert('The new ',x,1) \= 'CThe new REXX is faster than BREXX' then do
  errors=errors+1
  say 'INSERT failed in test 3' insert('The new ',x,1)
end

if insert(' ,isn"t it?',x,27) \= 'CREXX is faster than BREXX  ,isn"t it?' then do
  errors=errors+1
  say 'INSERT failed in test 4' insert(' ,isn"t it?',x,27)
end

/* These from TRL */

if insert(' ','abcdef',3) \= 'abc def' then do
    errors=errors+1
  say 'INSERT failed in test 5'
end

if insert('123','abc',5,6) \= 'abc  123   ' then do
    errors=errors+1
  say 'INSERT failed in test 6'
end

if insert('123','abc',5,6,'+') \= 'abc++123+++' then do
    errors=errors+1
  say 'INSERT failed in test 7'
end

/* if insert('123','abc') \= '123abc' then do */
/*     errors=errors+1 */
/*   say 'INSERT failed in test 8' */
/* end */

if insert('123','abc') \= '123abc'                then do
    errors=errors+1
  say 'INSERT failed in test 9'
end

if insert('123','abc',,5,'-') \= '123--abc'       then do
      errors=errors+1
  say 'INSERT failed in test 10'
end

/* These from Mark Hessling. */
if insert("abc","def") \=  "abcdef" then do
      errors=errors+1
  say 'INSERT failed in test 11'
end

if insert("abc","def",2) \=  "deabcf" then do
      errors=errors+1
  say 'INSERT failed in test 12'
end

if insert("abc","def",3) \=  "defabc" then do
      errors=errors+1
  say 'INSERT failed in test 13'
end

if insert("abc","def",5) \=  "def  abc" then do
      errors=errors+1
  say 'INSERT failed in test 14'
end

if insert("abc","def",5,,'*') \=  "def**abc" then do
      errors=errors+1
  say 'INSERT failed in test 15'
end

if insert("abc" ,"def",5,4,'*') \=  "def**abc*" then do
      errors=errors+1
  say 'INSERT failed in test 16'
end

if insert("abc","def",,0) \=  "def"       then do
      errors=errors+1
  say 'INSERT failed in test 17'
end

if insert("abc","def",2,1) \=  "deaf"     then do
      errors=errors+1
  say 'INSERT failed in test 18'
end

return errors<>0
