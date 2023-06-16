/* REXX Levelb VALUE Implementation */
options levelb

namespace rxfnsb expose value

/* Only does a read (and value is always returned as a string) for the initial release */

value: procedure = .string
  arg input = .string
say "===== VALUE ====="
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
     say 'max 'meta_array
     do i = 1 to meta_array
        assembler linkattr meta_entry,meta_array,i
        say i meta_entry meta_entry
        if meta_entry = ".meta_clear" then do /* Object type */
           assembler linkattr symbol,meta_entry,1
           say i "Clear Symbol "symbol
           if pos("."reg"@",symbol"@") > 0 then do /* TODO - Rough and ready find */
              say "A symbol found "symbol
              leave a
           end
        end

        else if meta_entry = ".meta_const" then do /* Object type */
           assembler linkattr symbol,meta_entry,1
            say i "Const Symbol "symbol
           if pos("."reg"@",symbol"@") > 0 then do /* TODO - Rough and ready find */
             assembler linkattr type,meta_entry,3
             assembler linkattr v,meta_entry,4
             result = v
             say "Result found "result
             leave a
           end
        end

        else if meta_entry = ".meta_reg" then do /* Object type */
           assembler linkattr symbol,meta_entry,1
            say i "Reg Symbol "symbol
           if pos("."reg"@",symbol"@") > 0 then do /* TODO - Rough and ready find */
              assembler linkattr type,meta_entry,3
              assembler linkattr r_num,meta_entry,4

              if type = ".int" then do
                 assembler metalinkpreg ires,r_num       /* Link parent-frame-register */
                 ires_copy = ires /* Don't want to alter ires with any side effects */
                 assembler unlink ires
                 result = ires_copy
                 say "int found "result
              end

              else if type = ".float" then do
                 assembler metalinkpreg fres,r_num       /* Link parent-frame-register */
                 fres_copy = fres
                 assembler unlink fres
                 result = fres_copy
                 say "float found "result
              end

              else do
                 assembler metalinkpreg sres,r_num       /* Link parent-frame-register */
                 sres_copy = sres
                 assembler unlink sres
                 result = sres_copy
                 say "str found "result
              end
           end
        end
     end
   end
  if result = "" then result = upper(input)
  return result
