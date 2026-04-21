options levelb

main: procedure = .int
  errors = .int
  i = .int
  j = .int
  f = .float
  d = .decimal

  errors = 0

  errors = errors + assert("const int eq string", 1 == "1", 1)
  errors = errors + assert("const int eq padded string", 1 == "01", 0)
  errors = errors + assert("const int lex gt", 2 >> 10, 1)
  errors = errors + assert("const int lex lt", 10 << 2, 1)
  errors = errors + assert("const no padding", "a" == "a ", 0)
  errors = errors + assert("const case", "A" == "a", 0)
  errors = errors + assert("const source form ignored", 01 == 1, 1)
  errors = errors + assert("const float eq string", 1.5 == "1.5", 1)
  errors = errors + assert("const decimal eq string", 1.5d == "1.5", 1)

  i = 2
  j = 10
  f = 1.5
  d = 1.5d
  errors = errors + assert("var int eq string", i == "2", 1)
  errors = errors + assert("var int lex gt", i >> j, 1)
  errors = errors + assert("var int lex lt", j << i, 1)
  errors = errors + assert("var float eq string", f == "1.5", 1)
  errors = errors + assert("var decimal eq string", d == "1.5", 1)

  errors = errors + assert("digits float", thirds_float_ok(), 1)
  errors = errors + assert("digits decimal", thirds_decimal_ok(), 1)

  if errors = 0 then say "strict compare ok"
  return errors <> 0

assert: procedure = .int
  arg label = .string, actual = .boolean, expected = .boolean
  if actual <> expected then do
    say "FAIL" label actual expected
    return 1
  end
  return 0

thirds_float_ok: procedure = .boolean
  numeric digits 5
  f = .float
  f = 1.0 / 3.0
  return f == "0.33333"

thirds_decimal_ok: procedure = .boolean
  numeric digits 5
  d = .decimal
  d = 1d / 3d
  return d == "0.33333"
