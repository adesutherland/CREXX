/************************************************************************
 *.  D 2 X   9.6.8
 ************************************************************************/

if \!Bif_ArgExists.2 then ArgData = 'rWHOLENUM>=0'
else ArgData = 'rWHOLENUM rWHOLE>=0'
call CheckArgs ArgData

/* Convert to manifest hexadecimal */
Subject = abs(!Bif_Arg.1 )
r = ReRadix(Subject,10,16)
/* Twos-complement for negatives */
if !Bif_Arg.1<0 then do
  Subject = 16**length(r)-Subject
  r = ReRadix(Subject,10,16)
end
if \!Bif_ArgExists.2 then return r
/* Adjust the length with appropriate characters. */
if !Bif_Arg.1>=0 then return right(r,!Bif_Arg.2,'0')
else return right(r,!Bif_Arg.2,'F')

