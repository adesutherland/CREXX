options levelc
numeric digits 5
numeric fuzz 2
signal on syntax name bad_digits
value='abc'
numeric digits value
say 'miss-digits'
exit
bad_digits:
say 'bad-digits' digits() fuzz()
signal on syntax name small_digits
numeric digits 2
say 'miss-small-digits'
exit
small_digits:
say 'small-digits' digits() fuzz()
signal on syntax name large_fuzz
numeric fuzz 5
say 'miss-large-fuzz'
exit
large_fuzz:
say 'large-fuzz' digits() fuzz()
signal on syntax name bad_fuzz
numeric fuzz '-1'
say 'miss-bad-fuzz'
exit
bad_fuzz:
say 'bad-fuzz' digits() fuzz()
numeric digits 6
numeric fuzz 3
say 'changed' digits() fuzz()
