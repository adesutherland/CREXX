/* rexx compile a rexx exec to a native executable      */
/* Classic Rexx (ooRexx, REgina) and NetRexx compatible */
crexx_home=directory()
if arg='' then do
  say 'exec name expected.'
  exit 99
end
parse arg execName'.'extension
if extension<>'' then say 'filename extension ignored.'
'rxc  -i' crexx_home'/lib/rxfnsb' execName
'rxas' execName 
'rxcpack' execName crexx_home'/lib/rxfnsb/library'
'gcc -o' execName,
'-lrxvml -lmachine -lavl_tree -lplatform -lm -L',
crexx_home'/interpreter -L'crexx_home'/machine -L',
crexx_home'/avl_tree -L'crexx_home'/platform'  execName'.c'
