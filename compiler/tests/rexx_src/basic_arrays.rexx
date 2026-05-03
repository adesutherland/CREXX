options levelb
s = .string[10]
do i = 1 to 3
  s[i] = "Item " || i
end

do i = 1 to 3
  say "Arr " || i || ": " || s[i]
end

/* Dotted index with variable */
idx = 4
s.idx = "Item 4"
say "Arr 4: " || s.4

/* Dotted index with constant */
s.5 = "Item 5"
say "Arr 5: " || s.5

return
