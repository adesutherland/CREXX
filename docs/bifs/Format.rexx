/************************************************************************
*.  F O R M A T   9.4.2
************************************************************************/
call CheckArgs,
'rNUM oWHOLE>=0 oWHOLE>=0 oWHOLE>=0 oWHOLE>=0'

if !Bif_ArgExists.2 then Before = !Bif_Arg.2
if !Bif_ArgExists.3 then After  = !Bif_Arg.3
if !Bif_ArgExists.4 then Expp   = !Bif_Arg.4
if !Bif_ArgExists.5 then Expt   = !Bif_Arg.5
else Expt   = !Digits.!Level
/* In the simplest case the first is the only argument. */
Number=!Bif_Arg.1
if !Bif_Arg.0 < 2 then return Number
/* Dissect the Number. It is in the normal Rexx format. */
parse var Number Mantissa 'E' Exponent
if Exponent == '' then Exponent = 0
Sign = 0
if left(Mantissa,1) == '-' then do
  Sign = 1
  Mantissa = substr(Mantissa,2)
end
/* We are interested in the value of the number, not the input format.
   So we can put it in the form of an integer Mantissa and an Exponent. */
parse var Mantissa Befo '.' Afte
Mantissa = Befo||Afte
Exponent=Exponent - length(Afte)
/* Leading zeros could have come from 0.01 for example. */
do while length(Mantissa)>1 & left(Mantissa,1)=='0'
  Mantissa=substr(Mantissa,2)
end
Point = length(Mantissa)
/* Sign Mantissa and Exponent now reflect the Number. */

/* Decide whether exponential form to be used, setting ShowExp. */
/* These tests have to be on the number before any rounding since
   decision on whether to have exponent affects what digits surround
   the decimal point. */
/* Only after this decision can Before and After arguments be checked. */
ShowExp = 0
/* There is a test about maximum number of digits in the part before
   the decimal point, if non-exponential is to be used. */
/* eg 123.4E+3 becomes 1234E+2 by point removal, 123400 non-floating */
if (length(Mantissa) + Exponent) > Expt then ShowExp = 1
/* Also a test on digits after the point. */
/* If the Exponent is negative at this point in the calculation there
   is a possibility that the non-exponential form would have too many
   zeros after the decimal point. */
   /* For classic this test is: */
   if !NotJ18 then if -Exponent > 2*Expt then
     if -Exponent >= length(Mantissa) then ShowExp = 1
     /* For modern it is: */
     /* The non-exponential value needs to be at least a millionth. */
     if \!NotJ18 then if (-Exponent - length(Mantissa)) >= 6 then ShowExp = 1
     /* An over-riding rule for exponential form: */
     if !Bif_ArgExists.4 then if Expp = 0 then ShowExp = 0
     
     /* ShowExp now indicates whether to show an exponent. */
     if ShowExp then do
       /* Construct the exponential part of the result. */
       /* Exponent at this point in the calculation reflects an integer
	  mantissa.  It has to be changed to reflect a decimal point at
	  Point from the left. */
       Point = 1 /* As required for 'SCIENTIFIC' */
       Exponent = Exponent + length(Mantissa) - 1
       if !Form.!Level == 'ENGINEERING' then
	 do while Exponent//3  \=  0
           Point = Point + 1
           Exponent = Exponent-1
           Mantissa=Mantissa'0'
         end
     end
     else do /* Force the exponent to zero for non-exponential format. */
       Point = length(Mantissa)
       do while Exponent>0
         Mantissa=Mantissa'0'
         Point=Point+1
         Exponent=Exponent-1
       end
       do while Exponent<0
         Point=Point-1
         if Point < 1 then do
           Mantissa='0'Mantissa
           Point = Point + 1
         end
         Exponent=Exponent+1
       end
       /* Now Exponent is Zero and Mantissa with Point reflects Number */
     end
     
     /* Deal with right of decimal point first since that can affect the
	left. Ensure the requested number of digits there. */
     Afters = length(Mantissa)-Point
     if !Bif_ArgExists.3 = 0 then After = Afters  /* Note default. */
     /* Make Afters match the requested After */
     do while Afters < After
       Afters = Afters+1
       Mantissa = Mantissa'0'
     end
     if Afters > After then do
       /* Round by adding 5 at the right place. */
       r=substr(Mantissa, Point + After + 1, 1)
       Mantissa = left(Mantissa, Point + After)
       MantLen = length(Mantissa)
       
       if r >= '5' then Mantissa = Mantissa + 1
       /* This can leave the result zero. */
       if Mantissa = 0 then Sign = 0
       /* This rounding could have made the mantissa shorter. */
       if length(Mantissa) < MantLen then
         Mantissa = copies('0',MantLen-length(Mantissa))||Mantissa
       /* The case when rounding makes the mantissa longer is an awkward
	  one. The exponent will have to be adjusted. */
       if length(Mantissa) > MantLen then do
         Point = Point+1
         if Point>Expt then ShowExp = 1
       end
       if ShowExp = 1 then do
         Exponent=Exponent + (Point - 1)
         Point = 1 /* As required for 'SCIENTIFIC' */
         if !Form.!Level == 'ENGINEERING' then
           do while Exponent//3  \=  0
             Point = Point+1
             Exponent = Exponent-1
           end
       end
     end /* Rounded */
     /* Right part is final now.  Set it into Afte */
     if After > 0 then Afte = '.'||substr(Mantissa,Point+1,After)
     else Afte = ''
     
     /* Now deal with the part of the result before the decimal point. */
     Mantissa = left(Mantissa,Point)
     if !Bif_ArgExists.2  =  0 then Before  =  Point + Sign /* Note default. */
     /* Make Point match Before */
     if Point > Before - Sign then call Raise  40.38, 2, !Bif_Arg.1
     do while Point<Before
       Point = Point+1
       Mantissa = '0'Mantissa
     end
     
     /* Find the Sign position and blank leading zeroes. */
     r = ''
     Triggered = 0
     do j = 1 to length(Mantissa)
       Digit = substr(Mantissa,j,1)
       /* Triggered is set when sign inserted or blanking finished. */
       if Triggered = 1 then do
         r = r||Digit
         iterate
       end
       /* If before sign insertion point then blank out zero. */
       if Digit = '0' then
         if substr(Mantissa,j+1,1) = '0' & j+1<length(Mantissa) then do
           r = r||' '
           iterate
         end
       /* j is the sign insertion point. */
       if Digit = '0' & j \= length(Mantissa) then Digit = ' '
       if Sign = 1 then Digit = '-'
       r = r||Digit
       Triggered = 1
     end j
     Number = r||Afte
     
     if ShowExp = 1 then do
       /* Format the exponent. */
       Expart = ''
       SignExp = 0
       if Exponent<0 then do
         SignExp = 1
         Exponent = -Exponent
       end
       /* Make the exponent to the requested width. */
       if !Bif_ArgExists.4 = 0 then Expp = length(Exponent)
       if length(Exponent) > Expp then
         call Raise 40.38, 4, !Bif_Arg.1
       Exponent=right(Exponent,Expp,'0')
       if Exponent = 0 then do
         if !Bif_ArgExists.4 then Expart = copies(' ',Expp+2)
       end
       else if SignExp = 0 then Expart = 'E+'Exponent
       else Expart = 'E-'Exponent
       Number = Number||Expart
     end
     return Number
     