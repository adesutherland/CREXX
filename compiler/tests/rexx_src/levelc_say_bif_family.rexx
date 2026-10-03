options levelc
x='MiXeD'
say 'length=' || length('ab')
say 'nested=' || length(reverse('abc'))
say 'substr=' || substr('abcdef', 2, 3)
say 'omitted=' || substr('abc', 2, , '.')
say 'pos=' || pos('bc', 'abcd')
say 'word=' || word('red blue', 2)
say 'value=' || value('x')
say 'local=' || inside()
say 'change=' || value('x', 'next')
say 'after=' || x
say 'symbol=' || symbol('x')
say 'x2d=' || x2d('FF')
say 'center=' || center('A', 3, '-')
say 'left=' || left('ab', 4, '.')
say 'min=' || min(4, 2)
say 'max=' || max(4, 2)
exit

inside: procedure expose x
return substr(value('x'), 2)
