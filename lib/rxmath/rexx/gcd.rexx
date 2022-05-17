/* GCD: Greatest Common Divisor    */
/* after Walter Pachl's gcd3       */
options levelb

gcd: procedure = .string
arg x = .string
x=abs(x)
do j=2 to arg()
  y=abs(arg(j))
  if y<>0 then do
    do until z==0
      z=x//y
      x=y
      y=z
    end
  end
end
return x

/* function prototype from abs() */
abs: procedure = .string
arg number = .string
