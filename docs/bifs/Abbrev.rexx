 /************************************************************************
 *.  A B B R E V  9.3.1                                                  *
 ************************************************************************/
 call CheckArgs 'rANY rANY oWHOLE>=0'
 
 Subject = !Bif_Arg.1
 Subj    = !Bif_Arg.2
 if !Bif_ArgExists.3 then Length = !Bif_Arg.3
 else Length = length(Subj)
 Cond1 = length(Subject) >= length(Subj)
 Cond2 = length(Subj) >= Length
 Cond3 = substr(Subject, 1, length(Subj)) == Subj
 return Cond1 & Cond2 & Cond3
