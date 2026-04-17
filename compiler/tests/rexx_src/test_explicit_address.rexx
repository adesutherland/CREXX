options levelb

main: procedure
  address system "echo EXPLICIT_OK1"
  cmd = "echo EXPLICIT_OK2"
  address system cmd
  return
