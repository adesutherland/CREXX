/* */
options levelb

say substr('1234567890','6',0,'')
say substr('1234567890','7',8,'.')

substr: procedure = .string
arg string1 = .string, start=.int, length=.int, pad=.string




