options levelb
line = "A"
old = ""
count = 0
do until old = line | line = ''
  count = count + 1
  old = line
  line = ''
end
say "Count: " || count
return 0
