/* rexx test abs bif */
options levelb
errors=0

x='the quick brown fox jumps over the lazy dog'
if reverse(x) \= 'god yzal eht revo spmuj xof nworb kciuq eht' then do
  errors=errors+1
  say 'REVERSE failed in test 1'
end

if reverse('god yzal eht revo spmuj xof nworb kciuq eht') \= 'the quick brown fox jumps over the lazy dog' then do
  errors=errors+1
  say 'REVERSE failed in test 2'
end

if reverse("") \= '' then do
  errors=errors+1
  say 'REVERSE failed in test 3'
end
  
return errors<>0

/* function prototype */
reverse: procedure = .string
arg string1 = .string
