options levelb
import inline_cross_file_dep

main: procedure
  say inc(41)
  say classify(-1)
  say classify(0)
  say classify(1)
  say scoped(2)
  say scoped(-3)
  say nested(20)
  looped = sumTo(4)
  say looped
  watched = countUntil(3)
  say watched
  values = buildArray("red", "blue")
  say renderArray(values)
  box = .inline_cross_file_dep..box("seed")
  same = identityBox(box)
  say same.getName()
  x = 10
  say refBump(x)
  say x
  say optAdd()
  say optAdd(3)
  say safeLength("abcdef")
  return
