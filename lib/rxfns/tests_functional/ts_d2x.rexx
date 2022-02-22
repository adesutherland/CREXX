/* D2X */
options levelb
errors=0
/* These from the Rexx book. */
if d2x(9) \= '9' then do
  errors=errors+1
  say 'D2X failed in test 1 '
end
if d2x(129) \= '81' then do
  errors=errors+1
  say 'D2X failed in test 2 d2x(129):' d2x(129) 'must be 81'
end
if d2x(129,1) \= '1' then do
  errors=errors+1
  say 'D2X failed in test 3 d2x(129,1)' d2x(129,1) 'must be 1'
end
if d2x(129,2) \= '81' then do
  errors=errors+1
  say 'D2X failed in test 4 d2x(129,2)' d2x(129,2) 'must be 81'
end
if d2x(129,4) \= '0081' then do
  errors=errors+1
  say 'D2X failed in test 5 d2x(129,4) ' d2x(129,4) 'must be 0081'
end
if d2x(257,2) \= '01' then do
  errors=errors+1
  say 'D2X failed in test 6 d2x(257,2)' d2x(257,2) 'must be 01'
end
if d2x(-127,2) \= '81' then do
  errors=errors+1
  say 'D2X failed in test 7 '
end
if d2x(-127,4) \= 'FF81' then do
  errors=errors+1
  say 'D2X failed in test 8 '
end
if d2x(12,0) \= '' then do
  errors=errors+1
  say 'D2X failed in test 9 '
end
/* These from Mark Hessling. */
/* if d2x(0) \= "" then do
errors=errors+1
   say 'D2X failed in test 10 '
   end */
if d2x(127) \= "7F" then do
  errors=errors+1
  say 'D2X failed in test 11 '
end
if d2x(128) \= "80" then do
  errors=errors+1
  say 'D2X failed in test 12 '
end
if d2x(129) \= "81" then do
  errors=errors+1
  say 'D2X failed in test 13 '
end
if d2x(1) \= "1" then do
  errors=errors+1
  say 'D2X failed in test 14 '
end
if d2x(-1,2) \= "FF" then do
  errors=errors+1
  say 'D2X failed in test 15 '
end
if d2x(-127,2) \= "81" then do
  errors=errors+1
  say 'D2X failed in test 16 '
end
if d2x(-128,2) \= "80" then do
  errors=errors+1
  say 'D2X failed in test 17 '
end
if d2x(-129,2) \= "7F" then do
  errors=errors+1
  say 'D2X failed in test 18 '
end
if d2x(-1,3) \= "FFF" then do
  errors=errors+1
  say 'D2X failed in test 19 d2x(-1,3)' d2x(-1,3) 'must be FFF'
end
if d2x(-127,3) \= "F81" then do
  errors=errors+1
  say 'D2X failed in test 20 d2x(-127,3)' d2x(-127,3) 'must be F81'
end
if d2x(-128,4) \= "FF80" then do
  errors=errors+1
  say 'D2X failed in test 21 '
end
if d2x(-129,5) \= "FFF7F" then do
  errors=errors+1
  say 'D2X failed in test 22 '
end
if d2x(129,0) \= "" then do
  errors=errors+1
  say 'D2X failed in test 23 '
end
if d2x(129,2) \= "81" then do
  errors=errors+1
  say 'D2X failed in test 24 '
end
if d2x(256+129,4) \= "0181" then do
  errors=errors+1
  say 'D2X failed in test 25 '
end
if d2x(256*256+256+129,6) \= "010181" then do
  errors=errors+1
  say 'D2X failed in test 26 '
end

return errors<>0

x2d: procedure = .int
  arg expose hex = .string, slen = -1

x2c: procedure = .string
  arg expose hex = .string

x2b: procedure = .string
  arg expose hex = .string, slen = -1

d2b: procedure = .string
  arg expose int1 = .int, slen = -1

d2x: procedure = .string
  arg expose int1 = .int, slen = -1

d2c: procedure = .string
  arg expose int1 = .int, slen = -1

c2x: procedure = .string
  arg expose string = .string


