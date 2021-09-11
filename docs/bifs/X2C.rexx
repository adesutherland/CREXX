 /************************************************************************
 *.  X 2 C   9.6.10
 ************************************************************************/
 call CheckArgs 'rHEX'
 
 Subject = !Bif_Arg.1
 if Subject == '' then return ''
 Subject = space(Subject,0)
 /* Convert to manifest binary */
 r = ReRadix(translate(Subject),16,2)
 /* Convert to character */
 Length = 8*((length(Subject)+1)%2)
 Indication = Config_B2C(right(r,Length,'0'))
 return !Outcome

