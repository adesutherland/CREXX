options levelb

main: procedure = .int
  say "Starting call-like arg inline expr test..."
  do i = 1 to 3
    box = .Box(addOne(i))
    say box.bump(addOne(i))
  end
  say "Call-like arg inline expr test finished."
  return 0

addOne: procedure = .int
  arg val = .int
  return val + 1

Box: class
  val = .int

  *: factory
    arg initialValue = .int
    val = initialValue
    return

  bump: method = .int
    arg amount = .int
    return val + amount
