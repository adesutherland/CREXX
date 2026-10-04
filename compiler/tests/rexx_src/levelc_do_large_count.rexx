options levelc
count='2147483648'
do count
  say 'dynamic'
  leave
end
do 2147483648
  say 'literal'
  leave
end
do j=1 for count
  say 'for' j
  leave
end j
do k=1 for 1E30
  say 'exponent' k
  leave
end k
say 'after' j k
