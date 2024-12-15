/* REXX Levelb SYMBOL Implementation */
options levelb

namespace rxfnsb expose symbol

/* Only does a read (and value is always returned as a string) for the initial release */

symbol: procedure = .string
arg inputsymbol = .string
  if inputsymbol = "" then return "BAD" /* ? */
  result = ""
  reg = lower(inputsymbol)
  symb = ""
  type = ""
  module = 0
  addr = 0
  meta_array = 0
  meta_entry = ""
  v = ""
  r_num = 0

  address_object = 0
  assembler metaloadcalleraddr address_object /* Address from where are we called */
  assembler linkattr1 module,address_object,1  /* 1 = Module number */
  assembler linkattr1 addr,address_object,2 /* 2 = Address in module */

  /* Read the addresses backwards */
  do i = addr to 0 by -1
     /* Get the metadata for that address */
     assembler metaloaddata meta_array,module,i
     do j = 1 to meta_array
        assembler linkattr1 meta_entry,meta_array,j
        if meta_entry = ".meta_clear" then do /* Object type */
           assembler linkattr1 symb,meta_entry,1
           if pos("."reg"@",symb"@") > 0 then leave
        end
        else if meta_entry = ".meta_const"| meta_entry = ".meta_reg"  then do /* Object type */
           assembler linkattr1 symb,meta_entry,1
           if pos("."reg"@",symb"@") > 0 then return 'VAR'
        end
     end
   end
  result = translate(inputsymbol,,'-+*/\:;')
  if result=inputsymbol then return 'LIT'
  return 'BAD'
