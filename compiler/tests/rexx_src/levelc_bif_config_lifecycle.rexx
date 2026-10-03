options levelc
first=random(1,100,123)
second=random(1,100)
marker='outer'
third=inside()
fourth=random(1,100)

repeat_first=random(1,100,123)
repeat_second=random(1,100)
repeat_third=random(1,100)
repeat_fourth=random(1,100)
say first=repeat_first
say second=repeat_second
say third=repeat_third
say fourth=repeat_fourth
say marker='outer'

call reseed
say second=random(1,100)
exit

inside: procedure
marker='inner'
return random(1,100)

reseed: procedure
unused=random(1,100,123)
return
