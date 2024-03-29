/* rexx */
options levelb
import rxfnsb

errors=0

a = 10
b = "hello"
aa = 10.5
d = a + 1
d = d + 0  /* Fool the rather braindead optimiser */
dd = aa + 1  /* Fool the rather braindead optimiser */
e = b "there"
e = e || "" /* Fool the rather braindead optimiser */
say 1 value('a')
if value('a') \= 10 then do
  errors=errors+1
  say 'VALUE failed in test      1'
end
say value('aa')
if value('aa') \= 10.5 then do
  errors=errors+1
  say 'VALUE failed in test      2'
end

say value('b')
if value('b') \= "hello" then do
  errors=errors+1
  say 'VALUE failed in test      3'
end

say value('c')
if value('c') \= "C" then do
  errors=errors+1
  say 'VALUE failed in test      4'
end

say value('d')
if value('d') \= 11 then do
  errors=errors+1
  say 'VALUE failed in test      5'
end

say value('dd')
if value('dd') \= 11.5 then do
  errors=errors+1
  say 'VALUE failed in test      6'
end

say value('e')
if value('e') \= "hello there" then do
  errors=errors+1
  say 'VALUE failed in test      7'
end

say value('f')
if value('f') \= "F" then do
  errors=errors+1
  say 'VALUE failed in test      8'
end

say value('g')
if value('g') \= "G" then do
  errors=errors+1
  say 'VALUE failed in test      9'
end

/* These are set AFTER the value() calls above */
f = 99
g = f + 1
g = g + 1

return errors<>0
