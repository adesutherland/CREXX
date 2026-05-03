options levelb
/* Nested Logic */
a = 10
b = 20
if a > 5 then do
    if b < 30 then say "A>5 B<30"
    else say "A>5 B>=30"
end
else say "A<=5"
