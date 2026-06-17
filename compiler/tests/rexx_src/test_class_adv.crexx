/* Advanced Class and Property Tests */
options levelb

say "Testing Advanced Classes and Properties..."

obj = .ComplexObj(5)

say "Static array [1]:" obj.getStatic(1)
say "Dynamic array [1]:" obj.getDyn(1)

call obj.chainA()
say "Final chain_val:" obj.getChainVal()

return 0

ComplexObj: class
  static_ary = .int[10]
  dyn_ary = .int[]
  chain_val = .int

  *: factory
    arg size = .int
    
    t_dyn = .int[50]
    dyn_ary = t_dyn
    
    static_ary[1] = 42
    dyn_ary[1] = 99
    
    chain_val = 0
    return

  getStatic: method = .int
    arg idx = .int
    return static_ary[idx]

  getDyn: method = .int
    arg idx = .int
    return dyn_ary[idx]

  getChainVal: method = .int
    return chain_val

  chainA: method = .void
    call chainB()
    return
    
  chainB: method = .void
    call chainC()
    return
    
  chainC: method = .void
    chain_val = 100
    say "Chained methods completed, chain_val =" chain_val
    return
