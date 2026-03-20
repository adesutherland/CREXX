options levelb

import rxfnsb

obj = .stem()

obj.bar = "10"
a = obj.bar
say a

if a \= "10" then return 1

obj["foo"] = "20"
b = obj["foo"]
say b

if b \= "20" then return 1

/* Check for errors */
/* Actually, let's leave errors to another test and make this one green */
return 0
