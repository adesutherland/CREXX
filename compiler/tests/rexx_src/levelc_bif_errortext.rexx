options levelc
numeric digits 2
say errortext(40)
say errortext('040.24','standard')
say errortext(40.34,'normal')
say length(errortext(0)) length(errortext(90.9))
say length(errortext('40.240'))
signal on syntax name bad
say errortext(91)
exit
bad:
  say rc sigl condition('c')
  say condition('d')
  exit
