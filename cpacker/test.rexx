/* Test REXX Program */
options levelb
string =  "René Vincent Jansen, Welcome to cREXX!"
do i = 1 to words(string)
 say word(string, i)
end
return 0

word: procedure = .string
  arg string1 = .string, int2 = .int

words: procedure = .int
  arg string1 = .string