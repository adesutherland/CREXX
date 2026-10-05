options levelc
signal on syntax name caught
say substr('abc','')
exit
caught:
say condition('D') = 'Error 40.12: SUBSTR argument 2 must be a whole number; found ""'
return
