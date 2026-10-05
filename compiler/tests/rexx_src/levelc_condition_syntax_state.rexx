options levelc
signal on syntax name caught
say substr('abc', 0)
exit
caught:
say 'name=' || condition('C')
say 'code=' || condition('E')
say 'description=' || left(condition('D'), 11)
say 'instruction=' || condition()
say 'state=' || condition('S')
say 'rc=' || rc
