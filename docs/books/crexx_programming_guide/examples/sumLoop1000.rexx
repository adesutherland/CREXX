/* compute sum of numbers 1 to 100 (5050) */
options levelb
/* compute sum of numbers 1 to 100000  */
sum = 0
do i=1 to 100000
   sum = i+sum
end
say "the sum of the numbers 1 to 100000 is:" sum
return
