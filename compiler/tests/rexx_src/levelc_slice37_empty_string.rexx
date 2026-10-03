options levelc
say ''
say ""
empty=''
len1=length(empty)
len2=length("")
say 'length' len1 len2
say 'joined' 'left' || '' || 'right'
result=echo('')
say 'call' result
say 'compare' (empty=='')
exit
echo: procedure
arg value
return length(value)
