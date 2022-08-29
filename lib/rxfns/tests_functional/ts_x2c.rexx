/* rexx */
options levelb
import rxfnsb

/* X2C */
errors=0
/* These from the Rexx book. */
/* if x2c('F7F2 A2') \= '72s' then say 'X2C failed in test EBCDIC */
/* if x2c('F7F2a2') \= '72s' then do
   errors=errors+1
   say 'X2C failed in test EBCDIC */
if tx2c('F') \= '0f' then do
  errors=errors+1
  say 'X2C failed in test 3 '
end
/* These from Mark Hessling. */
/* if x2c("416263") \= "Abc" then do
   errors=errors+1
   say 'X2C failed in test 4 ' probably also EBCDIC */
if tx2c("DeadBeef") \= "deadbeef" then do
  errors=errors+1
  say 'X2C failed in test 5 '
end
if tx2c("1 02 03") \= "010203" then do
  errors=errors+1
  say 'X2C failed in test 6 '
end
if tx2c("11 0222 3333 044444") \= "1102223333044444" then do
  errors=errors+1
  say 'X2C failed in test 7 '
end
if tx2c("") \= "" then do
  errors=errors+1
  say 'X2C failed in test 8 '
end
if tx2c("2") \= "02" then do
  errors=errors+1
  say 'X2C failed in test 9 '
end
if tx2c("1 02 03") \= "010203" then do
  errors=errors+1
  say 'X2C failed in test 10 '
end

return errors<>0

tx2c: procedure = .string
arg hex = .string
slen=0
str=x2c(hex)
str2=c2x(str)  /* 2 half bytes translated into 2 bytes */
assembler strlen slen,str2
if slen//2=1 then str2='0'str2
return str2
