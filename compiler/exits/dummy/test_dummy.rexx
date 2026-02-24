options levelb
import rxfnsb

main: procedure
  /* Outer variable */
  i = .int
  i = 99

  /* Call the dummy exit which will inject a typed local 'i' inside EXIT_OWNED */
  dummy i

  /* After the exit, the outer 'i' must remain 99 if EXIT_OWNED scoping works */
  say 'after-exit ' || i
  return
