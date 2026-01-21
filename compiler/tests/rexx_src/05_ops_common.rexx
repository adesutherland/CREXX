options levelb numeric_common
/* Expect C-like precedence: -3**2 == -9 */
res1 = -3**2

/* Expect Right-Associative Power: 2**2**3 == 2**(8) == 256 */
res2 = 2**2**3

say res1 res2
