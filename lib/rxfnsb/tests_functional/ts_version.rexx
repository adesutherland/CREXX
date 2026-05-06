/* rexx */
options levelb
import rxfnsb

v = version()
if pos("crexx-1.0.0-beta.2", v) = 0 then do
  say "version() did not contain crexx-1.0.0-beta.2:" v
  return 1
end

say "PASS: version helper"
return 0
