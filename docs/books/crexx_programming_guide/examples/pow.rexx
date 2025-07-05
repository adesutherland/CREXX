/* crexx ipow test - inline assembler */
options levelb

number = 10
power = 2
result = 0

say 'test IPOW'
assembler do
   ipow result,number,power
end
say "result =" result /* The compiler converts integer to string */