/* Rexx test a sqlite call */
options levelb
import rxfnsb

query_in = .string[]
query_in.1 = 'select *',
             'from inst_name'

query_out = .string[]
address cmd "sqlite3 instructionbase.sqb" input query_in output query_out

say '# of output records:' query_out.0

do i=1 to query_out.0
  say i query_out.i
end
