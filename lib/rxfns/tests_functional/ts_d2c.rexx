/* D2C */
options levelb
errors=0
say "Look for D2C OK"
/* These from the Rexx book. */
if td2c(9)      \= '09'  then do
  errors=errors+1
  say 'D2C failed in test 1 '
end
if td2c(129)    \= '81'  then do
  errors=errors+1
  say 'D2C failed in test 2 '
end
if td2c(129,1)  \= '81'   then do
  errors=errors+1
  say 'D2C failed in test 3 '
end
if td2c(129,2)  \= '0081'  then do
  errors=errors+1
  say 'D2C failed in test 4 '
end
if td2c(257,1)  \= '01'   then do
  errors=errors+1
  say 'D2C failed in test 5 '
end
if td2c(-127,1) \= '81'   then do
  errors=errors+1
  say 'D2C failed in test 6 '
end
if td2c(-127,2) \= 'ff81' then do
  errors=errors+1
  say 'D2C failed in test 7 '
end

if td2c(-1,4) \= 'ffffffff' then do
  errors=errors+1
  say 'D2C failed in test 8 '
end

if td2c(12,0) \= '' then do
  errors=errors+1
  say 'D2C failed in test 9 '
end
/* These from Mark Hessling. */
if td2c(127) \= "7f"  then do
  errors=errors+1
  say 'D2C failed in test 10 '
end

if td2c(128) \= "80"  then do
  errors=errors+1
  say 'D2C failed in test 11 '
end

if td2c(129) \= "81"  then do
  errors=errors+1
  say 'D2C D2C failed in test 12 '
end

if td2c(1) \= "01"  then do
  errors=errors+1
  say 'D2C failed in test 13 '
end

if td2c(-1,1) \= "ff"  then do
  errors=errors+1
  say 'D2C failed in test 14 '
end

if td2c(-127,1) \= "81"  then do
  errors=errors+1
  say 'D2C failed in test 15 '
end

if td2c(-128,1) \= "80"  then do
  errors=errors+1
  say 'D2C failed in test 16 '
end

if td2c(-129,1) \= "7f" then do
  errors=errors+1
  say 'D2C failed in test 17 '
end

if td2c(-1,2) \= "ffff" then do
  errors=errors+1
  say 'D2C failed in test 18 '
end

if td2c(-127,2) \= "ff81" then do
  errors=errors+1
  say 'D2C failed in test 19 '
end

if td2c(-128,2) \= "ff80" then do
  errors=errors+1
  say 'D2C failed in test 20 '
end

if td2c(-129,2) \= "ff7f" then do
  errors=errors+1
  say 'D2C failed in test 21 '
end

if td2c(129,0) \= "" then do
  errors=errors+1
  say 'D2C failed in test 22 '
end

if td2c(129,1) \= "81" then do
  errors=errors+1
  say 'D2C failed in test 23 '
end

if td2c(256+129,2) \= "0181" then do
  errors=errors+1
  say 'D2C failed in test 24 '
end

if td2c(256*256+256+129,3) \= "010181" then do
  errors=errors+1
 say 'D2C failed in test 25 '
end

return

td2c: procedure = .string
  arg hex = .int, len=-1
  slen=0
  str=d2c(hex,len)
  str2=c2x(str)  /* 2 half bytes translated into 2 bytes */
  assembler strlen slen,str2
  if slen//2=1 then str2='0'str2
  say "d2c result '"str2"'"
return str2



x2d: procedure = .int
  arg hex = .string, slen = -1

x2c: procedure = .string
  arg hex = .string

x2b: procedure = .string
  arg hex = .string, slen = -1

d2b: procedure = .string
  arg int1 = .int, slen = -1

d2x: procedure = .string
  arg int1 = .int, slen = -1

d2c: procedure = .string
  arg int1 = .int, slen = -1

c2x: procedure = .string
  arg string = .string


