/* rexx compile a rexx exec to a native executable           */
/* Classic Rexx (ooRexx, REgina), NetRexx and CREXX (when    */
/* ran as a simple script from the crexx wrapper) compatible */

parse arg crexx_home execSpec
crexx_home = strip(crexx_home)
execSpec = strip(execSpec)

if crexx_home='' | execSpec='' then do
  say 'usage: crxc.rexx crexx_home execName[.ext]'
  exit 99
end

parse var execSpec execName '.' extension
if extension<>'' then say 'filename extension ignored.'

address system '"' || crexx_home || '/rxc" -i "' || crexx_home || '" ' || execName
if rc<>0 then exit rc
address system '"' || crexx_home || '/rxas" ' || execName
if rc<>0 then exit rc
address system '"' || crexx_home || '/rxcpack" ' || execName || ' ' || crexx_home || '/library'
if rc<>0 then exit rc
address system 'gcc -o ' || execName || ' -L "' || crexx_home || '" -lrxvml -lavl_tree -lplatform -lm ' || execName || '.c'
if rc<>0 then exit rc
exit 0
