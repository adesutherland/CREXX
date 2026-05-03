options levelb
/* Untyped assignment inside DO creates block-local when no outer exists */
main: procedure
  do
    y = 3
  end
  say y  /* expect 'y' - taken [as a] constant */
  return
