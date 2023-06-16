/* REXX Levelb VALUE Implementation */
options levelb

namespace rxfnsb expose value

/* Only does a read (and value is always returned as a string) for the initial release */

value: procedure = .string
  arg input = .string

  if input = "" then return "BAD" /* ? */
  result = ""
  ires = 0
  fres = 0.0
  sres = ""
  reg = lower(input)
  symbol = ""
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
  do a = address to 0 by -1
     /* Get the metadata for that address */
     assembler metaloaddata meta_array,module,a
     do i = 1 to meta_array
        assembler linkattr meta_entry,meta_array,i
        say i meta_entry
        if meta_entry = ".meta_clear" then do /* Object type */
           assembler linkattr symbol,meta_entry,1
           if pos("."reg"@",symbol"@") > 0 then return "VAR"
        end
    end
  end
  if result = "" then result = upper(input)
  return result
