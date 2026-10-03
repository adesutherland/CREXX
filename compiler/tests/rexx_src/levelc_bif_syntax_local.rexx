options levelc
say 'before'
call inside
say 'after'
exit

inside: procedure
say substr('abc', 0)
return
