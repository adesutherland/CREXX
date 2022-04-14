/* rexx test abs bif */
options levelb
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

return errors<>0

/* function prototype */
insert: procedure = .string
  arg expose insstr = .string, expose string = .string, position = .int, len = 0, pad = ' '



