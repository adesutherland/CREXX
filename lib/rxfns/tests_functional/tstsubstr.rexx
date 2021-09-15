/* */
options levelb

say substr('1234567890',5)
say substr('1234567890',6,6,'.')
say substr('abc',2)
say substr('abc',2,4)
say substr('abc',2,4,'.')
say substr('abcdefgh',1,2,'.')
say substr('abcdefgh',2,3,'é')

substr: procedure = .string
arg string1 = .string, start=.int, length=.int, pad=.string
