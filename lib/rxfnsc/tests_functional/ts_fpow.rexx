/* ipow Test */
options levelb

parm1 = 2.71828183
parm2 = 3.14159265
result = 0.0  /* So that the compiler knows result is an float */

say 'test FPOW'
assembler do
   fpow result,parm1,parm2
end
say "result =" result