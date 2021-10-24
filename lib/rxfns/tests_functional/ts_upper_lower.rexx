/* rexx */
options levelb

  st1=upper("The quick brown fox jumps over the lazy dog")
  say "upper: ""'"st1"'"
  st2=lower(st1)
  say "Lower: ""'"st2"'"
return

/* lower()  */
lower: procedure = .string
  arg expose string1 = .string

/* upper()  */
upper: procedure = .string
  arg expose string1 = .string
