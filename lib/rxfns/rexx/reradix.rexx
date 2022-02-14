/*
 *.  ReRadix
 */ 
options levelb
reradix: procedure = .string
arg Subject = .string, FromRadix = .int, ToRadix = .int
/* Converts Arg(1) from radix Arg(2) to radix Arg(3) */
/* Radix range is 2-16.  Conversion is via decimal   */
Subject = upper(Subject)
 Integer=0
 do j=1 to length(Subject)
   /* Individual digits have already been checked for range. */
   Integer=Integer*FromRadix+pos(substr(Subject,j,1),'0123456789ABCDEF')-1
   /* This test not for standard. */
    if pos('E',Integer)>0 then do
      say "ReRadix unable"
      qm = '?'
      return 999999999999999999999999
    end
 end
 r=''
 if Integer=0 then r='0'
 do while Integer>0
    r=substr('0123456789ABCDEF',1+Integer//ToRadix,1)||r
   Integer=Integer%ToRadix
 end
 /* When between 2 and 16, there is no zero suppression. */
 if FromRadix=2 & ToRadix=16 then
   r=right(r,(length(Subject)+3)%4,'0')
 else if FromRadix=16 & ToRadix=2 then
   r=right(r,length(Subject)*4,'0')
 return r

/* Length() Procedure */
length: procedure = .int
  arg string1 = .string

/* Substr() Procedure */
substr: procedure = .string
  arg string1 = .string, start = .int, len = length(string1) + 1 - start, pad = ' '

/* function prototype */
right: procedure = .string
arg string1 = .string, length = .int, pad = ' '

 /* function prototype */
pos: procedure = .int
arg string1 = .string, string2 = .string, start = 1

/* upper()  */
upper: procedure = .string
  arg expose string1 = .string
