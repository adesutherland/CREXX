options levelb
outbuf = .string[10]
lino = 0
lino = lino + 1
outbuf.lino = "Line A"
lino = lino + 1
outbuf.lino = "Line B"
say "Count: " || lino
say "Buf 1: " || outbuf.1
say "Buf 2: " || outbuf.2
return 0
