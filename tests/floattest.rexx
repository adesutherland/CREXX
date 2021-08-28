/* */
options levelb
max = 21
u = 2.0
v = -4.0

do i=3 to max
  w = 111.0 - 1130.0/v + 3000.0/(v * u)
  u = v
  v = w
end

say v