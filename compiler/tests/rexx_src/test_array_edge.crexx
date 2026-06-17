/* Array Edge Cases and Lifecycle Tests */
options levelb

say "Testing Array Edge Cases..."

/* Resizing/Reassignment */
ary = .int[]
ary[1] = 10
ary[5] = 50

say "ary[1]:" ary[1] "ary[5]:" ary[5]

/* Reassign to larger array */
ary = .int[]
ary[1] = 100
ary[10] = 1000

say "new ary[1]:" ary[1] "ary[10]:" ary[10]

/* Array of Objects */
objAry = .MyClass[3]
objAry[1] = .MyClass(1)
objAry[2] = .MyClass(2)
objAry[3] = .MyClass(3)

say "objAry[1].val =" objAry[1].getval()
say "objAry[2].val =" objAry[2].getval()
say "objAry[3].val =" objAry[3].getval()

return 0

MyClass: class
  val = .int

  *: factory
    arg v = .int
    val = v
    return

  getval: method = .int
    return val
