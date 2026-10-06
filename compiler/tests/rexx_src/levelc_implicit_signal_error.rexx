options levelc
signal on error name trapped
address bogus
'missing'
exit 50
trapped:
say 'signal=' || condition('C') || '/' || condition('D') || '/' || sigl
say 'status=' || rc || '/' || .rc || '/' || .rs || '/' || address()
