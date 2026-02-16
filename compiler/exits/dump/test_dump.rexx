options levelb
import rxfnsb

main: procedure
x = 100
y = "Hello World"

/* The exit should rewrite this to: say 'x=' || x; say 'y=' || y; */
address cms dump x y
