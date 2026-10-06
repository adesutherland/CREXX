options levelc
result = 'previous'
call last
say 'resumed=' || result
exit 0
last:
say 'last'
