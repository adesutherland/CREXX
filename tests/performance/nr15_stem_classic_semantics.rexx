/* NR-15 focused Classic Rexx comparison: valid for Regina and ooRexx. */
errors = 0

drop item.
if item.missing <> "ITEM.MISSING" then errors = fail(errors, "missing before default")
item.alpha = "alpha-one"
item. = "default-one"
if item.alpha <> "default-one" then errors = fail(errors, "default replaces existing")
if item.missing <> "default-one" then errors = fail(errors, "missing after default")

left = "customer"
middle = "2026.07"
right = "total"
item.left.middle.right = "149.95"
if item.left.middle.right <> "149.95" then errors = fail(errors, "dynamic multi-tail")

number = 42
item.number = "integer-tail"
if item.42 <> "integer-tail" then errors = fail(errors, "integer tail conversion")

item.a = "a0"
item.b = "b0"
item. = "d1"
item.a = "a1"
item. = "d2"
item.b = "b2"
item. = "d3"
item.a = "a3"
if item.a <> "a3" | item.b <> "d3" | item.c <> "d3" then
  errors = fail(errors, "repeated default and selective writes")

drop item.a
if item.a <> "ITEM.A" then errors = fail(errors, "drop compound tail becomes uninitialized")
drop item.
if item.a <> "ITEM.A" then errors = fail(errors, "drop whole stem removes default")

if errors = 0 then say "PASS: NR-15 Classic stem semantic comparison"
exit errors <> 0

fail: procedure
  parse arg errors, label
  say "FAIL: NR-15 Classic" label
  return errors + 1
