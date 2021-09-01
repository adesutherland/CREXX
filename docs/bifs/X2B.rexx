 /************************************************************************
 *.  X 2 B   9.6.9
 ************************************************************************/
 call CheckArgs 'rHEX'
 
 Subject = !Bif_Arg.1
 if Subject == '' then return ''
 /* Blanks were checked by CheckArgs, here they are ignored. */
 Subject = space(Subject,0)
 return ReRadix(translate(Subject),16,2)

