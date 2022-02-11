/* rexx */
options levelb
/* VERIFY */
errors=0
/* These from the Rexx book. */
if verify('123','1234567890') \= 0 then do
  errors=errors+1
  say 'VERIFY failed in test 1 '
end
if verify('1Z3','1234567890') \= 2 then do
  errors=errors+1
  say 'VERIFY failed in test 2 '
end
if verify('AB4T','1234567890','M') \= 3 then do
  errors=errors+1
  say 'VERIFY failed in test 3 '
end
if verify('1P3Q4','1234567890',,3) \= 4 then do
  errors=errors+1
  say 'VERIFY failed in test 4 '
end
if verify('ABCDE','',,3) \= 3 then do
  errors=errors+1
  say 'VERIFY failed in test 5 '
end
if verify('AB3CD5','1234567890','M',4) \= 6 then do
  errors=errors+1
  say 'VERIFY failed in test 6 '
end
  /* These from Mark Hessling. */
if verify('foobar', 'barfo', N, 1) \= 0 then do
  errors=errors+1
  say 'VERIFY failed in test 7 '
end
if verify('foobar', 'barfo', M, 1) \= 1 then do
  errors=errors+1
  say 'VERIFY failed in test 8 '
end
if verify('', 'barfo') \= 0 then do
  errors=errors+1
  say 'VERIFY failed in test 9 '
end
if verify('foobar', '') \= 1 then do
  errors=errors+1
  say 'VERIFY failed in test 10 '
end
if verify('foobar', 'barf', N, 3) \= 3 then do
  errors=errors+1
  say 'VERIFY failed in test 11 '
end
if verify('foobar', 'barf', N, 4) \= 0 then do
  errors=errors+1
  say 'VERIFY failed in test 12 '
end
if verify('', '') \= 0 then do
  errors=errors+1
  say 'VERIFY failed in test 13 '
end
/* say verify('123456','0123456789') */
/* say verify('12w36','0123456789') */
/* say verify('12w36','0123456789',,4) */
/* say verify('a12w36','0123456789','M') */
/* return */
/* verify()  */
return errors<>0

verify: procedure = .int
  arg instring = .string, intab = .string, match = 'N',pos=1
