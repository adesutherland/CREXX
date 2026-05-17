options levelb comments_dash
import data_qfind
import rxfnsb

line = '"is=name"=Peter'
line2 = 'is=name=Peter'
say "Test 1"
m = .matchResult(line, "=")

if m.found()=1 then do
  say m.before()   /* name */
  say m.match()    /* = */
  say m.after()    /* Peter */
end

say "Test 2"
m = .matchResult(line2, "=")

if m.found()=1 then do
  say m.before()   /* name */
  say m.match()    /* = */
  say m.after()    /* Peter */
end

