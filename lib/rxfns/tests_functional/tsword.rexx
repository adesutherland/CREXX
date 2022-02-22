/* WORD */
options levelb
errors=0
/* These from the Rexx book. */
if word('Now is the time',3) \= 'the' then do
  errors=errors+1
  say 'WORD failed in test 1 '
end
if word('Now is the time',5) \= '' then do
  errors=errors+1
  say 'WORD failed in test 2 '
  /* These from Mark Hessling. */
end
if word('This is certainly a test',1) \= 'This' then do
  errors=errors+1
  say 'WORD failed in test 3 '
end
if word(' This is certainly a test',1) \= 'This' then do
  errors=errors+1
  say 'WORD failed in test 4 '
end
if word('This is certainly a test',1) \= 'This' then do
  errors=errors+1
  say 'WORD failed in test 5 '
end
if word('This is certainly a test',2) \= 'is' then do
  errors=errors+1
  say 'WORD failed in test 6 '
end
if word('This is certainly a test',2) \= 'is' then do
  errors=errors+1
  say 'WORD failed in test 7 '
end
if word('This is certainly a test',5) \= 'test' then do
  errors=errors+1
  say 'WORD failed in test 8 '
end
if word('This is certainly a test ',5) \= 'test' then do
  errors=errors+1
  say 'WORD failed in test 9 '
end
if word('This is certainly a test',6) \= '' then do
  errors=errors+1
  say 'WORD failed in test 10 '
end
if word('',1) \= '' then do
  errors=errors+1
  say 'WORD failed in test 11 '
end
if word('',10) \= '' then do
  errors=errors+1
  say 'WORD failed in test 12 '
end
if word('test ',2) \= '' then do
  errors=errors+1
  say 'WORD failed in test 13 '
end
return errors<>0
/* function prototype */
word: procedure = .string
arg string1 = .string, number = .int
