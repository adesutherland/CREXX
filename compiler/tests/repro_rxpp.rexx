options levelb
outbuf = .string[10]
lino = 1
text = "Line 1"
outbuf[lino] = text
lino = lino + 1
outbuf[lino] = "Line 2"
say "Count: " || lino
say "Buf 1: " || outbuf[1]
say "Buf 2: " || outbuf[2]
return 0
