/* /\* rexx test abs bif *\/ */
options levelb
import rxfnsb

/* x='the quick brown fox jumps over the lazy dog' */
/* wrds=words(x) */
/* do i=1 to wrds */
/*    say word(x,i) wordlength(x,i) */
/* end */
/* return */
/* WORDLENGTH */
errors=0
/* These from the Rexx book. */
if wordlength('Now is the time',2) \=2 then do
  errors=errors+1
  say 'WORDLENGTH failed in test 1 '
end
if wordlength('Now comes the time',2) \=5 then do
  errors=errors+1
  say 'WORDLENGTH failed in test 2 '
end
if wordlength('Now is the time',6) \=0 then do
  errors=errors+1
  say 'WORDLENGTH failed in test 3 '
end
/* These from Mark Hessling. */
if wordlength('This is certainly a test',1) \= '4' then do
  errors=errors+1
  say 'WORDLENGTH failed in test 4 '
end
if wordlength('This is certainly a test',2) \= '2' then do
  errors=errors+1
  say 'WORDLENGTH failed in test 5 '
end
if wordlength('This is certainly a test',5) \= '4' then do
  errors=errors+1
  say 'WORDLENGTH failed in test 6 '
end
if wordlength('This is certainly a test ',5) \= '4' then do
  errors=errors+1
  say 'WORDLENGTH failed in test 7 '
end
if wordlength('This is certainly a test',6) \= '0' then do
  errors=errors+1
  say 'WORDLENGTH failed in test 8 '
end
if wordlength('',1) \= '0' then do
  errors=errors+1
  say 'WORDLENGTH failed in test 9 '
end
if wordlength('',10) \= '0' then do
  errors=errors+1
  say 'WORDLENGTH failed in test 10 '
end
return errors<>0
