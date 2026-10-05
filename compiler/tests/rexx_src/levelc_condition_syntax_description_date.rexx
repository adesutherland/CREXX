options levelc
signal on syntax name caught
say date('B','nonsense','S')
exit
caught:
say condition('D') = 'Error 40.19: DATE argument 2, "nonsense", is not in the format described by argument 3, "S"'
return
