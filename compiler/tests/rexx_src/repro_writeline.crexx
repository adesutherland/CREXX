options levelb
namespace rxpp expose outbuf lino
outbuf = .string[]
lino = 1

call writeline "Line 1"
call writeline "Line 2"

say "Lino: " || lino
say "Buf 2: " || outbuf.2
return

writeline: procedure
  arg oline=.string
  lino = lino + 1
  outbuf.lino = oline
return
