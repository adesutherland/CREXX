/* rexx test abs bif */
options levelb
/* RXVM ts_date date _jdn _dateo _datei abbrev right word words wordindex wordpos pos substr length copies upper */
say "test Time"


say "Option '' "time()
say "Option N  "time("N")
say "Option L  "time("l")
say "Option C  "time("C")
say "Option H  "time("h")
say "Option M  "time("M")
say "Option S  "time("s")

return


/* Prototype functions */
time: Procedure = .string
   arg option = ""

