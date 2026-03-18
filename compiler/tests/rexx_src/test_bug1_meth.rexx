options levelb

c = .MyClass()
call c.mymeth()
return 0

MyClass: class
  keys = .string[10]

  *: factory
    keys = .string[10]
    return

  mymeth: method
    keys[1] = "hello"
    say keys[1]
    return
