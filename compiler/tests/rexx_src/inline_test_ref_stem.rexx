options levelb
namespace refstem expose outbuf lino

say "Starting stem ref inline test..."
outbuf = .string[]
outbuf[2] = "untouched"
lino = 1

call fillSlot(outbuf.lino)
say outbuf[1]
say outbuf[2]
say "Stem ref inline test finished."
return 0

fillSlot: procedure = .void
  arg expose slot = .string
  lino = lino + 1
  slot = "slot-one"
  return
