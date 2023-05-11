options levelb
A=3
DAY='Monday'

say '    Start #: Precedence'
failed = 0
if -3**2 <> 9 then do
  say 'test 1 failed'
  failed = failed + 1
end

say A+5 ' should be 8'
say A-4*2 ' should be -5'
say A/2 ' should be 1.5'
say 0.5**2 'should be 0.25'
say (A+1)>7 'should be 0'
say ' '='' 'should be 1'
/* TODO say ' '=='' 'should be 0' */
/* say ' '\=='' 'should be 1' */
say (A+1)*3 'should be 12'
say Today is DAY 'should be TODAY IS Monday'
say 'If it is' day 'should be If it is Monday'
/* say Substr(Day,2,3) */
say '!'xxx'!' 'should be !XXX!'
say 'abc' << 'abd' 'should be 1'
say '077' >> '11' 'should be 0' 
say 'abc' >> 'ab' 'should be 1'
say 'ab ' << 'abd' 'should be 1'

say -3**2 'should be 9'
say -(2+1)**2 'should be 9'
say 2**2**3 'should be 64'
