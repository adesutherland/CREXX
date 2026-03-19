/* Methods vs. Procedures Integration Tests */
options levelb

import rxfnsb

say "Testing Methods vs Procedures..."

obj = .MyObj("initial")
say "Initial:" obj.getval()

call testProc obj
say "After testProc:" obj.getval()

res = obj.passThrough("hello", "world")
say "passThrough:" res

shad = obj.shadowing("shadow_arg")
say "shadowing:" shad
say "After shadowing val:" obj.getval()

return 0

testProc: procedure
  arg obj = .MyObj
  call obj.modifySelf("changed_in_proc")
  return

MyObj: class
  val = .string

  *: factory
    arg v = .string
    val = v
    return

  modifySelf: method = .void
    arg newVal = .string
    val = newVal
    return

  getval: method = .string
    return val

  passThrough: method = .string
    arg arg1 = .string, arg2 = .string
    /* Test register preservation across internal method call */
    l = internalLen(arg1)
    /* If registers are trampled, arg2 might be lost */
    return arg1 || "_" || arg2 || "_" || l

  internalLen: method = .int
    arg s = .string
    return length(s)

  shadowing: method = .string
    arg val = .string
    /* The argument 'val' should shadow property 'val' */
    return val
