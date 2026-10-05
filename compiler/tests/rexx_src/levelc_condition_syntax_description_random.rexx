options levelc
signal on syntax name caught
say random(10,1)
exit
caught:
say condition('D') = 'Error 40.33: RANDOM argument 1 ("10") must be less than or equal to argument 2 ("1")'
return
