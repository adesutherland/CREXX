options levelb

main: procedure
  address command "echo EXPLICIT_OK1"
  address path "echo EXPLICIT_OK2"
  cmd = "echo EXPLICIT_OK3"
  address cmd cmd
  address shell "echo EXPLICIT_OK4"
  return
