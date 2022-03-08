/* rexx */
/* These from TRL */
options levelb
errors=0
if copies('abc',3) \= 'abcabcabc' then do
  errors=errors+1
  say 'COPIES failed in test 1 '
end
if copies('abc',0) \= '' then do
  errors=errors+1
  say 'COPIES failed in test 2 '
  /* These from Mark Hessling. */
end
if copies("foo",3) \= "foofoofoo" then do
  errors=errors+1
  say 'COPIES failed in test 3 '
end
if copies("x", 10) \= "xxxxxxxxxx" then do
  errors=errors+1
  say 'COPIES failed in test 4 '
end
if copies("", 50) \= "" then do
  errors=errors+1
  say 'COPIES failed in test 5 '
end
if copies("", 0) \= "" then do
  errors=errors+1
  say 'COPIES failed in test 6 '
end
if copies("foobar",0 ) \= "" then do
  errors=errors+1
  say 'COPIES failed in test 7 '
end
return errors<>0

/* function prototype */
copies: procedure = .string
arg string1 = .string, times= .int
