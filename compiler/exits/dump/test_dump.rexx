options levelb
import rxfnsb
/* This test verifies the dump exit */
x = 100
y = "Hello World"

/* The exit should rewrite this to: say 'x=' || x; say 'y=' || y; */
address cms dump x y
