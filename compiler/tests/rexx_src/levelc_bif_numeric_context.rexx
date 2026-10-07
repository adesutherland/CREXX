options levelc
numeric digits 2
say abs('-1.2345')
say max('1.2345','1.2346')
say min('1.2345','1.2346')
say sign('-0.00012345')
say trunc('12.3456',3)
say format('12.3456',,3)
call nested
say digits() form()
numeric digits 9
say format('1.20')
say max('1.0','1.00') min('1.00','1.0')
exit
nested:
  procedure
  numeric digits 5
  numeric form engineering
  say abs('-1234567') max('1234567','1234544')
  say trunc('12.3456',3) format('12.3456',,3)
  return
