/* rexx */
options levelb
import rxfnsb

errors=0
mary=10
bert="Fred"
J=3
/* following: Drop A.3; J=3 */
if  SYMBOL('J') <> 'VAR' then do /* -> 'VAR' */
  errors=errors+1
  say 'SYMBOL failed in test      1'
end

if SYMBOL(J) <> 'LIT' then do /* -> 'LIT' /\* has tested "3" *\/ */
  errors=errors+1
  say 'SYMBOL failed in test      2'
end

/* if SYMBOL('a.j') /\* -> 'LIT' /\\* has tested A.3 *\\/ *\/ */

if SYMBOL(2) <> 'LIT' then do /* -> 'LIT' /\* a constant symbol *\/ */
    errors=errors+1
  say 'SYMBOL failed in test      3'
end

if SYMBOL('*') <> 'BAD' then do /* -> 'BAD' /\* not a valid symbol *\/ */
  errors=errors+1
  say 'SYMBOL failed in test      4'
end

if symbol('mary') <> 'VAR' then do
  errors=errors+1
  say 'SYMBOL failed in test      5'
end

if symbol('bert') <> 'VAR' then do 
  errors=errors+1
  say 'SYMBOL failed in test      6'
end

if symbol('cccc') <> 'LIT' then do
  errors=errors+1
  say 'SYMBOL failed in test      7'
end

if symbol('cc*cc') <> 'BAD' then do 
    errors=errors+1
  say 'SYMBOL failed in test      8'
end

return errors<>0