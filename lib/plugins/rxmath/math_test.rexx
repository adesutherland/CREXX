/* GETPI Plugin Test */
options levelb
import rxmath
import rxfnsb

say "DJB2 Hash"
say djb2("Hello, world!")
say djb2("The quick brown fox jumps over the lazy dog")
say djb2("Waltz, bad nymph, for quick jigs vex")
say djb2("Glib jocks quiz nymph to vex dwarf")
say djb2("Sphinx of black quartz, judge my vow")
say djb2("How quickly daft jumping zebras vex!")
say djb2("The five boxing wizards jump quickly")
say djb2("Jackdaws love my big sphinx of quartz")
say djb2("Pack my box with five dozen liquor jugs")
say djb2("Victor jagt zwölf Boxkämpfer quer über den großen Sylter Deich")
say djb2("Carthago delenda est, pax, lux, cum quaesivi Romam")
say djb2("Jupiter ex Urania Phobo vexat cadens lux")
say djb2("Xenos e filos hedista lambanei charan yper zonis")
say djb2("Zeus kai Hera, Apollon kai Artemis, Hermes kai Athena phylassousi to chaos")
say "Murmur Hash"
say murmur("Hello, world!",42)
say murmur("The quick brown fox jumps over the lazy dog",4711)
say murmur("Waltz, bad nymph, for quick jigs vex",4711)
say murmur("Glib jocks quiz nymph to vex dwarf",4711)
say murmur("Sphinx of black quartz, judge my vow",4711)
say murmur("How quickly daft jumping zebras vex!",4711)
say murmur("The five boxing wizards jump quickly",4711)
say murmur("Jackdaws love my big sphinx of quartz",4711)
say murmur("Pack my box with five dozen liquor jugs",4711)
say murmur("Victor jagt zwölf Boxkämpfer quer über den großen Sylter Deich",4711)
say murmur("Carthago delenda est, pax, lux, cum quaesivi Romam",4711)
say murmur("Jupiter ex Urania Phobo vexat cadens lux",4711)
say murmur("Xenos e filos hedista lambanei charan yper zonis",4711)
say murmur("Zeus kai Hera, Apollon kai Artemis, Hermes kai Athena phylassousi to chaos",4711)
say "fnv1a"
say fnv1a("Hello, world!")

say "CRC32"
say crc32("Hello, world!")
say "Trigonometry"
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
   nn.i=i*2.0+7
end


say 'mean   1 'mean(mm)
say 'stddev 1 'stddev(mm)
say 'mean   2 'mean(nn)
say 'stddev 2 'stddev(nn)

say 'covar   'covar(mm,nn)
say 'correl  'correl(mm,nn)

slope=-99.0
yaxis=-99.0
say 'regression  'regression(mm,nn,slope,yaxis)
say 'slope       'slope
say 'Y-intercept 'yaxis