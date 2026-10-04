options levelc
say c2x('FF'x)
say c2x('11111111'b)
say c2x('C3A9'x)
say length('C3A9'x)
say c2x('00'x || 'FF'x)
say c2x('F'x)
say c2x('111'b)
say c2x('C3 A9'x)
value = '80'x
say c2x(value)
say c2x(identity('80'x))
source = 'A' || 'C3A9'x || 'B'
parse var source first 'C3A9'x last
say first || last
source = 'A' || 'FF'x || 'B'
parse var source first '11111111'b last
say first || last
exit

identity: procedure
  arg item
  return item
