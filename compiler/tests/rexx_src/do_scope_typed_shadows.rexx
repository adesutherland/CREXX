options levelb
/* DO block variable shadowing with typed declaration */
main: procedure
  x = .int
  x = 1
  do
    x = .int  /* should create block-local x that shadows outer x */
    x = 9
  end
  say x  /* expect 1 after block ends */
  return
