options levelb
/* DO UNTIL */
main: procedure
  ready = .int
  ready = 0
  do until ready = 1
    ready = 1
  end
  return
