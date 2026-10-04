options levelc
failures = 0
do ordinal = 0 to 255
  hex = d2x(ordinal, 2)
  if c2x(x2c(hex)) <> hex then failures = failures + 1
end
say failures
say c2x(x2c('FF'))
say c2d(x2c('FF'))
say c2x(d2c(255))
say c2x(bitxor(x2c('E9'), x2c('FF')))
say c2x(xrange(x2c('FE'), x2c('00')))
say length(x2c('FF'))
