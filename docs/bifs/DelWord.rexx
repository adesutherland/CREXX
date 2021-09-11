/************************************************************************
 *.  D E L W O R D  9.3.8
 ************************************************************************/
call CheckArgs 'rANY rWHOLE>0 oWHOLE>=0'

String = !Bif_Arg.1
Num    = !Bif_Arg.2
if !Bif_ArgExists.3 then Len = !Bif_Arg.3

if Num > words(String) then return String

EndLeft = wordindex(String, Num) - 1
Output = left(String, EndLeft)
if !Bif_ArgExists.3 then do
  BeginRight = wordindex(String, Num + Len)
  if BeginRight>0 then
    Output = Output || substr(String, BeginRight)
end
return Output

