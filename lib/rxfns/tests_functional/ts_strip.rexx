/* rexx */
options levelb

/* say "'"strip("   The quick brown fox jumps over the lazy dog  ")"'" */
/* say "'"strip("   The quick brown fox jumps over the lazy dog  ",'L')"'" */
/* say "'"strip("   The quick brown fox jumps over the lazy dog  ",'t')"'" */
/* say "'"strip("-----The quick brown fox jumps over the lazy dog---",,'-')"'" */
/* say "'"strip("-----The quick brown fox jumps over the lazy dog---",'B','-')"'" */
/* say "'"strip("-----The quick brown fox jumps over the lazy dog---",'l','-')"'" */
/* say "'"strip("-----The quick brown fox jumps over the lazy dog---",'t','-')"'" */

/* STRIP */
errors=0
/* These from the Rexx book. */
if strip(' ab c ') \= 'ab c' then do
  errors=errors+1
  say 'STRIP failed in test 1 '
end
if strip(' ab c ','L') \= 'ab c ' then do
  errors=errors+1
  say 'STRIP failed in test 2 '
end
if strip(' ab c ','t') \= ' ab c' then do
  errors=errors+1
  say 'STRIP failed in test 3 '
end
if strip('12.7000',,0) \= '12.7' then do
  errors=errors+1
  say 'STRIP failed in test 4 '
end
if strip('0012.7000',,0) \= '12.7' then do
  errors=errors+1
  say 'STRIP failed in test 5 '
end
/* These from Mark Hessling. */
if strip(" foo bar ") \= "foo bar" then do
  errors=errors+1
  say 'STRIP failed in test 6 '
end
if strip(" foo bar ",'L') \= "foo bar " then do
  errors=errors+1
  say 'STRIP failed in test 7 '
end
if strip(" foo bar ",'T') \= " foo bar" then do
  errors=errors+1
  say 'STRIP failed in test 8 '
end
if strip(" foo bar ",'B') \= "foo bar" then do
  errors=errors+1
  say 'STRIP failed in test 9 '
end
if strip(" foo bar ",'B','*') \= " foo bar " then do
  errors=errors+1
  say 'STRIP failed in test 10 '
end
if strip(" foo bar",,'r') \= " foo ba" then do
  errors=errors+1
  say 'STRIP failed in test 11 '
end

return errors<>0

/* strip()  */
strip: procedure = .string
  arg string1 = .string, option='B', tchar=" "

