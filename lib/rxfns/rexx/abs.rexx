options levelb

abs: procedure = .string
arg number = .string
if left(number,1) = '-' then number = substr(number,2)
return number

substr: procedure = .string
arg string1 = .string, start = .int, len = length(string1) + 1 - start, pad = ' '
  
left: procedure = .string
arg string = .string, length1 = .int, pad = '0'

length: procedure = .int
  arg string1 = .string
