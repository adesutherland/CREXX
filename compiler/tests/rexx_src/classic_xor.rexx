options levelb numeric_classic

say 0 && 0
say 0 && 1
say 1 && 0
say 1 && 1

/* This distinguishes logical XOR from ordinary not-equals under Level B's
   boolean target conversion path. */
say 2 && 3
