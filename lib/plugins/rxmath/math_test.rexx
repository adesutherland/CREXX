/* GETPI Plugin Test */
options levelb
import rxmath
import rxfnsb
say "ACOS "acos(0)
say "ASIN "asin(0)
say "ATAN "atan(90)
say "COS  "cos (90)
say "COSH "cosh(90)
say "EXP  "exp (90)
say "EXP2 "exp2(90)
say "LOG "log (90)
say "LOG10 "log10(90)
say "LOG2 "log2(90)
say "pow10 "pow10(3)
say "pow "pow(2,3)

say 'CEIL 'ceil(pi())
say 'floor 'floor(pi())
say 'fabs  'fabs(-pi())
say 'round 'round(-pi())
say 'fmod  'fmod(-pi(),3)

say "SIN  "sin (90)
say "SINH "sinh(90)
say "SQRT "sqrt(16)
say "CBRT "cbrt(3*3*3)
say "TAN  "tan (90)
say "TANH "tanh(90)
say "POW10 "pow10(3)
say "PI "pi()
say "Euler "euler()

say "erf "erf(10)
say "erfc "erfc(10)
say "tgamma "tgamma(10)
say "lgamma "lgamma(11)

say "asinh "asinh(10)
say "acosh "acosh(50)
say "atanh "atanh(0)
say "hypot "hypot(4,10)

do i=1 to 5000
   mm.i=i*1.0
   nn.i=i*2.0
end


say 'mean   1 'mean(mm)
say 'stddev 1 'stddev(mm)
say 'mean   2 'mean(nn)
say 'stddev 2 'stddev(nn)

say 'covar   'covar(mm,nn)
say 'correl  'correl(mm,nn)
