/* REXX Levelb SYMBOL Implementation */
options levelb

namespace rxfnsb expose symbol

/* Only does a read (and value is always returned as a string) for the initial release */

symbol: procedure = .string
arg input = .string
  if input = "" then return "BAD" /* ? */
  result = ""
  reg = lower(input)
  symb = ""
  type = ""
  module = 0
  address = 0
  meta_array = 0
  meta_entry = ""
  v = ""
  r_num = 0

  address_object = 0
  assembler metaloadcalleraddr address_object /* Address from where are we called */
  assembler linkattr module,address_object,1  /* 1 = Module number */
  assembler linkattr address,address_object,2 /* 2 = Address in module */

  /* Read the addresses backwards */
  do i = address to 0 by -1
     /* Get the metadata for that address */
     assembler metaloaddata meta_array,module,i
     do j = 1 to meta_array
        assembler linkattr meta_entry,meta_array,j
        if meta_entry = ".meta_clear" then do /* Object type */
           assembler linkattr symb,meta_entry,1
           if pos("."reg"@",symb"@") > 0 then leave
        end
        else if meta_entry = ".meta_const"| meta_entry = ".meta_reg"  then do /* Object type */
           assembler linkattr symb,meta_entry,1
           if pos("."reg"@",symb"@") > 0 then return 'VAR'
        end
     end
   end
  result = translate(input,,'-+*/\:;')
  if result=input then return 'LIT'
  return 'BAD'
