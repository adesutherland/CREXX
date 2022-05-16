options levelb
/* used to check a customs client number ("eori")
 * which must adhere to the BSN-form of the modulo-11 test
 */

y = 'NL003787643'
y = substr(y,3)
if checkBSN(y,11) = 0 then
  do
    say 'NL'y 'passed'
    return 0
  end
else do
  say y 'failed'
  return 1
end
/* loop i=0 to 9 */
/* yy = y||i */
/* if m.checkBSN(yy,11) == 0 then */
/*   do */
/*     say 'NL'yy 'passed with generated check digit' */
/*     leave */
/*   end */
/* else iterate */
/* end */
    
checkBSN: procedure = .string
arg in=.string, modulo=.string
sum = 0
num= reverse(in)
l=length(in)
do i=1 to l
  if i=1 then
    do
      sum = sum+substr(num,i,1)*(-1)
    end
  else do
    sum = sum+substr(num,i,1)*(i)
  end
end
return sum//modulo

/* function prototypes */
substr: procedure = .string
arg string1 = .string, start = .int, len = length(string1) + 1 - start, pad = ' '

reverse: procedure = .string
arg string1 = .string

length: procedure = .int
arg string1 = .string




  