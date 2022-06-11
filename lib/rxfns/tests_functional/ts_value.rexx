/* rexx */
options levelb
errors=0

a = 10
b = "hello"
d = a + 1
d = d + 0  /* Fool the rather braindead optimiser */
e = b "there"
e = e || "" /* Fool the rather braindead optimiser */

if value('a') \= 10 then do
  errors=errors+1
  say 'VALUE failed in test      1'
end

if value('b') \= "hello" then do
  errors=errors+1
  say 'VALUE failed in test      2'
end

if value('c') \= "C" then do
  errors=errors+1
  say 'VALUE failed in test      3'
end

if value('d') \= 11 then do
  errors=errors+1
  say 'VALUE failed in test      4'
end

if value('e') \= "hello there" then do
say "value(e) is '" || value('e') || '"'
  errors=errors+1
  say 'VALUE failed in test      5 (e is "' || e || '")'
end

if value('f') \= "F" then do
  errors=errors+1
  say 'VALUE failed in test      6'
end

if value('g') \= "G" then do
  errors=errors+1
  say 'VALUE failed in test      7'
end

/* These are set AFTER the value() calls above */
f = 99
g = f + 1
g = g + 1

return errors<>0

/* value()  */
value: procedure = .string
  arg input = .string
