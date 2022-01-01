/* rexx */
options levelb
/* upper(string) translate to upper cases */
lower: procedure = .string
  arg expose string = .string
  newstr=""
  assembler strlower newstr,string
return newstr
