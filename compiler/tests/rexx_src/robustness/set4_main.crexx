options levelb
import set4_mid
import rxfnsb

say "Call mid_func:" mid_func()

/* Shadowing test - local assignment */
mid_func = "Local Shadow"
say "mid_func variable:" mid_func

/* word() is a BIF. Should not be shadowed by accidental early placeholder */
say "Word 2 of 'A B C' is:" word("A B C", 2)

/* Explicit shadow of BIF */
word = "Shadowed BIF"
say "word variable:" word

return 0
