options levelb
/* This test verifies the dump exit */
x = 100
y = "Hello World"

/* The exit should rewrite this to: say 'x=' || x; say 'y=' || y; */
dump x y
