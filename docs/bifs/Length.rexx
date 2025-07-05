 /************************************************************************
 *.  L E N G T H   9.3.12
 ************************************************************************/
 call CheckArgs 'rANY'
 
 String = !Bif_Arg.1
 
 Indication = Config_Length(String)
 Length = !Outcome
 call Config_Substr Indication, 1
 if !Outcome \== 'E' then return Length
 /* Here if argument was not a character string. */
 call Config_C2B String
 call !Raise 'SYNTAX', 23.1, b2x(!Outcome)
 /* No return to here */

    