options levelb
import rxfnsb

errors = 0
s = .stem()

if s.missing \= "" then do
  errors = errors + 1
  say "Stem default mismatch:" s.missing
end

s.alpha = "one"
s["beta"] = "two"
s.gamma = "three"

if s.alpha \= "one" then do
  errors = errors + 1
  say "Stem dotted lookup mismatch:" s.alpha
end

if s["beta"] \= "two" then do
  errors = errors + 1
  say "Stem bracket lookup mismatch:" s["beta"]
end

s.alpha = "updated"
if s.alpha \= "updated" then do
  errors = errors + 1
  say "Stem update mismatch:" s.alpha
end

if errors = 0 then say "PASS: stem object access"
return errors <> 0
