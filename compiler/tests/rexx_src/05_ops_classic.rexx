options levelb numeric_classic
/* Expect Classic precedence: -3**2 == (-3)**2 == 9 */
res1 = -3**2

/* Expect Left-Associative Power: 2**2**3 == (2**2)**3 == 64 */
res2 = 2**2**3

say res1 res2
