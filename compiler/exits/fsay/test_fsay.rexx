options levelb
import rxfnsb

main: procedure

 name[1]  = "Fred"
 qty[1]   = 3
 price[1]= 12.45

 name[2]   = "Alice"
 qty[2]    = 15
 price[2]  = 3.11

 name[3]   = "Bob"
 qty[3]    = 7
 price[3]  = 98.75

 name[4]   = "Charlotte"
 qty[4]   = 102
 price[4]  = 1.05

 name[5]   = "Dave"
 qty[5]    = 1
 price[5] = 250.99

 do i=1 to name[0]
    total = qty.i * price.i
    fsay "{i:>3} {name.i:<12} {qty.i:>5} {price.i:>6.2} Total={total:>6.2}"
 end