/* least common multiple */
/* Walter Pachl          */

options levelb

lcm: procedure = .string
arg x= .string
abs(x)
do k=2 to arg() While x<>0
  y=abs(arg(k))
  x=x*y/gcd(x,y)
  end
return x

abs: procedure = .string
arg number = .string

gcd: procedure = .string
arg x = .string

