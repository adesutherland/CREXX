options levelc
signal on syntax name caught
say substr('abc', 0)
exit
caught:
say condition('D') = 'Error 40.14: SUBSTR argument 2 must be positive; found "0"'
return
