/* rexx test center bif */
options levelb
say "Random(,,4711)     "random(,,4711)
say "Random()           "random()
say "Random()           "random()
say "Random()           "random()
say "Random()           "random()
say "Random(,555555)    "random(,555555)
say "Random(,,18601)    "random(,,1860)
say "Random(,300,31415) "random(,300,31415926)
say "Random(10,300,314) "random(10,300,314)
say "Random(10,300,314) "random(10,300,314)
say "Random(10,300,314) "random(10,300,314)
say "Random(10,5,123)   "random(10,5,123)
return 0

random: procedure = .int
arg expose from=0, rto=-1,seed = -1
