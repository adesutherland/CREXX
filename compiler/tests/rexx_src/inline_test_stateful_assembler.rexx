options levelb

s = "abc123"
x = char_at(s, 4)
y = char_at(s, 1)
say x':'y':'s

s2 = "abcdef"
z = left_copy(s2, 3)
say z':'s2

char_at: procedure = .string
  arg text = .string, posn = .int
  offset = posn - 1
  one = 1
  out = .string
  assembler setstrpos text, offset
  assembler substring out, text, one
  return out

left_copy: procedure = .string
  arg text = .string, len = .int
  assembler substcut text, len
  return text
