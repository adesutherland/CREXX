options levelb
  len=0
  result=""
  pos=12
  cut=10
  cut2=16
  cut3=255

  B="the quick brown fox jumps over the lazy dog"
  assembler substring result,b,pos  /* copy string from offset pos to result */
    say "'"result"'"
  assembler substcut result,cut     /* cut off string at position cut  */
    say "'"result"'"
  assembler substcut result,cut2     /* setting a new length show string still exists */
    say "'"result"'"
  assembler substcut result,cut3     /* setting a new length show string still exists */
    say "'"result"'"

return
