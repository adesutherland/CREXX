/* rexx */
options levelb
errors=0

a = 10
b = "hello"

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

return errors<>0

/* value()  */
value: procedure = .string
  arg input = .string
