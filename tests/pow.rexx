/* ipow Test */
options levelb
parm1 = 10
parm2 = 3
result = 0 /* So that the compiler knows result is an integer */

say 'test IPOW'
assembler do
   imult result,parm1,parm2
end
say "result =" result /* The compiler can do the say - converting integer to string automatically */