/*
   Prototype CREXX Debugger
   Not complete but showing concepts around the introspection meta-instructions

   Version 0.1 - and that is being kind!
*/
options levelb

say "RXDB Version 0.1"
say ""
say "Loading CREXX Runtime Library Modules"
call load_library "../cmake-build-debug-mingw/lib/rxfns"

last_loaded_module = ""

do forever
    cmd = ""
    say "Non-running state command (h=help):"
    assembler readline cmd

    c1 = word(cmd,1)
    c2 = word(cmd,2)

    if cmd = 'h' then do
       say 'help:'
       say '  q        - Quit'
       say '  e        - Print exposed procedures in module to be debugged (last module loaded)'
       say '  a        - Print exposed procedures (all modules)'
       say '  r proc   - Run procedure {proc}'
       say '  l rxbin  - Load rxbin module file {rxbin}'
    end
    else if cmd = 'q' then do
      leave
    end
    else if cmd = 'e' then do
      modules = 0
      assembler metaloadedmodules modules /* get the last module number */
      call dump_procs modules
    end
    else if cmd = 'a' then do
      call dump_procs 0
    end
    else if c1 = 'l' then do
      if c2 = "" then say "File name missing"
      else do
        mod_num = load_module(c2)
        if mod_num > 0 then do
          say "Module" c2 "loaded #"mod_num
          last_loaded_module = c2
        end
      end
    end
    else if c1 = 'r' then do
      if c2 = "" then c2 = last_loaded_module
      if c2 = "" then say "Procedure name missing"
      else do
        call_id = find_proc(0,c2)
        if call_id = 0 then do
          c2 = c2".main"
          call_id = find_proc(0,c2)
        end
        if call_id = 0 then say "Procedure" c2 "not found"
        else do
          rc = 0
          no_args = 0
          say "RXDB:" c2 "starting"
          assembler bpon
          assembler dcall rc,call_id,no_args
          assembler bpoff
          say "RXDB:" c2 "returned"
        end
      end
    end
    else say 'unknown command'
end

say "RXDB Exiting"
return

/* This is the interrupt handler that is called before every rxas instruction */
/* Note that interrupts are automatically disabled */
stephandler: procedure = .int
  arg expose address_object = .int;
  cmd = ""
  address = 0
  module = 0
  r = 0
  assembler linkattr module,address_object,1 /* 1 = Module number */
  assembler linkattr address,address_object,2 /* 2 = Address in module */

  /* Are we the last module */
  modules = 0
  assembler metaloadedmodules modules
  if modules <> module then return /* Don't debug the debugger */
/*
  esc = x2c('1B')
  red = esc"[31m"
  reset = esc"[0m"
*/
  do forever
    say ">"
    call print_asm module,address

    say "Running state command (ENTER=step, h=help):"
    assembler readline cmd

    c1 = word(cmd,1)

    if cmd = '' then leave
    else if cmd = 'h' then do
       say 'help:'
       say '  ENTER    - Step to next instruction'
       say '  q        - Quit'
       say '  p n ...  - Print regs n ... (space delimited list of numbers)'
       say '  e        - Print exposed procedures in module being debugged (last module loaded)'
       say '  a        - Print exposed procedures (all modules)'
    end
    else if cmd = 'q' then do
      assembler exit
    end
    else if cmd = 'e' then do
      call dump_procs module
    end
    else if cmd = 'a' then do
      call dump_procs 0
    end
    else if c1 = 'p' then do
      do i = 2
        reg = word(cmd,i)
        if reg = "" then leave
        if isint(reg) then do
          value = ""
          r = reg
          ires = 0
          fres = 0.0
          sres = ""
          assembler metalinkpreg ires,r       /* Link parent-frame-register */
          ires_copy = ires /* Don't want to alter ires with any side effects */
          assembler unlink ires
          value = "int=" || ires_copy

          assembler metalinkpreg fres,r       /* Link parent-frame-register */
          fres_copy = fres
          assembler unlink fres
          value = value "float=" || fres_copy

          assembler metalinkpreg sres,r       /* Link parent-frame-register */
          sres_copy = sres
          assembler unlink sres
          value = value "string='" || sres_copy || "'"

          say "r" || r":" value
        end
      end
    end
    else say 'unknown command'
  end
  say ""
  call print_asm module,address
  return

/* This disassembles the code at address */
print_asm: procedure = .int
  arg module = .int, address = .int

  opcode = 0;               /* Holds the opcode at the address */
  instruction = ""          /* Mnemonic */
  description = ""          /* Instruction Description */
  no_operands = 0           /* Number of operands */
  op1_type = 0              /* Operand Types */
  op2_type = 0
  op3_type = 0
  instruction_object = 0    /* Set to 0 as the compiler doesn't understand objects yet! */

  /* Load the opcode from the address */
  assembler metaloadinst opcode,module,address

  /* Decode the opcode populating the instruction_object */
  assembler metadecodeinst instruction_object,opcode

  /* Get the instruction mnemonic attribute */
  /* This makes the instruction variable link/point to the attribute */
  assembler linkattr instruction,instruction_object,2 /* 2 = instruction/mnemonic */

  /* Get the instruction description attribute */
  assembler linkattr description,instruction_object,3 /* 3 = description */

  /* Get the number of operands */
  assembler linkattr no_operands,instruction_object,4 /* 4 = number of operands (0-3) */

  /* Get the operand types */
  assembler linkattr op1_type,instruction_object,5 /* 5 = operand 1 type */
  assembler linkattr op2_type,instruction_object,6 /* 6 = operand 2 type */
  assembler linkattr op3_type,instruction_object,7 /* 7 = operand 3 type */

  /* Job Done */
  say " " right(d2x(opcode),3,"0") "@" right(d2x(module),3,"0")":"right(d2x(address),4,"0") instruction opdesc(op1_type,module,address+1)","opdesc(op2_type,module,address+2)","opdesc(op3_type,module,address+3) "*" description

  /* Unlink the variables - not strictly necessary as they are all local and we are returning */
  assembler unlink instruction
  assembler unlink description
  /* Note we can't unlink no_operands as we are about to return it! */

  return no_operands

opdesc: procedure = .string
  arg code = .int, module = .int, address = .int
  desc = "unknown"
  int_val = 0
  float_val = 0.0
  string_val = ""

  if code = 0 then desc = ""
  else if code = 1 then do
    assembler metaloadioperand int_val,module,address
    desc = "@" || right(d2x(int_val),4,'0')
  end
  else if code = 2 then do
    assembler metaloadioperand int_val,module,address
    desc = "r" || int_val
  end
  else if code = 3 then do
    assembler metaloadpoperand string_val,module,address
    desc = string_val || "()"
  end
  else if code = 4 then do
    assembler metaloadioperand int_val,module,address
    desc = int_val
  end
  else if code = 5 then do
    assembler metaloadfoperand float_val,module,address
    desc = float_val || "f"
  end
  else if code = 6 then do
    assembler metaloadioperand int_val,module,address
    desc = int_val || "c"
  end
  else if code = 7 then do
    assembler metaloadsoperand string_val,module,address
    desc = '"' || string_val || '"'
  end

  return desc

/* Load a module */
load_module: procedure = .int
   arg module_name = .string

   mod_num = 0
   assembler metaloadmodule mod_num, module_name
   if mod_num = 0 then do
     say "Error Loading module" module_name
   end

   return mod_num


/* Print procedures in a module or all modules if mod_num = 0 */
dump_procs: procedure = .int
  arg mod_num = .int

  modules = 0 /* Compiler doesn't understand arrays yet */
  module = ""
  proc_name = ""
  proc_id = 0
  proc = 0
  procs = 0

  assembler metaloadedmodules modules

  if mod_num = 0 then do
    say "There are" modules "modules"
    start = 1
    finish = modules
  end
  else do
    start = mod_num
    finish = mod_num
  end

  do i = start to finish
    assembler linkattr module,modules,i
    say "Module" i module
    assembler metaloadedprocs procs,i
    do j = 1 to procs
      assembler linkattr proc,procs,j
      assembler linkattr proc_name,proc,1
      assembler linkattr proc_id,proc,2
      say "Procedure" proc_name '@' proc_id
    end
  end
  return


/* Find procedure of a name in a module */
find_proc: procedure = .int
  arg mod_num = .int, name = .string

  modules = 0 /* Compiler doesn't understand arrays yet */
  module = ""
  proc_name = ""
  proc_id = 0
  proc = 0
  procs = 0
  found_id = 0

  assembler metaloadedmodules modules
  if mod_num = 0 then do
    start = 1
    finish = modules
  end
  else do
    start = mod_num
    finish = mod_num
  end

  do mod_num = start to finish
    assembler linkattr module,modules,mod_num
    assembler metaloadedprocs procs,mod_num
    do j = 1 to procs
      assembler linkattr proc,procs,j
      assembler linkattr proc_name,proc,1
      assembler linkattr proc_id,proc,2
      if proc_name = name then do
        return proc_id
      end
    end
  end
  return 0

load_library: procedure = .int
  arg dir = .string
  call load_module dir || "/rexx/substr"
  call load_module dir || "/rexx/length"
  call load_module dir || "/rexx/raise"
  call load_module dir || "/rexx/linesize"
  call load_module dir || "/rexx/abbrev"
  call load_module dir || "/rexx/x2d"
  call load_module dir || "/rexx/x2c"
  call load_module dir || "/rexx/x2b"
  call load_module dir || "/rexx/d2b"
  call load_module dir || "/rexx/d2c"
  call load_module dir || "/rexx/d2x"
  call load_module dir || "/rexx/c2x"
  call load_module dir || "/rexx/c2d"
  call load_module dir || "/rexx/center"
  call load_module dir || "/rexx/changestr"
  call load_module dir || "/rexx/delstr"
  call load_module dir || "/rxas/right"
  call load_module dir || "/rxas/left"
  call load_module dir || "/rxas/copies"
  call load_module dir || "/rxas/_elapsed"
  call load_module dir || "/rxas/pos"
  call load_module dir || "/rxas/lastpos"
  call load_module dir || "/rxas/reverse"
  call load_module dir || "/rexx/centre"
  call load_module dir || "/rexx/compare"
  call load_module dir || "/rexx/countstr"
  call load_module dir || "/rexx/insert"
  call load_module dir || "/rexx/lower"
  call load_module dir || "/rexx/min"
  call load_module dir || "/rexx/max"
  call load_module dir || "/rexx/date"
  call load_module dir || "/rexx/time"
  call load_module dir || "/rexx/_datei"
  call load_module dir || "/rexx/_dateo"
  call load_module dir || "/rexx/_jdn"
  call load_module dir || "/rexx/_itrunc"
  call load_module dir || "/rexx/_ftrunc"
  call load_module dir || "/rexx/delword"
  call load_module dir || "/rexx/wordlength"
  call load_module dir || "/rexx/wordpos"
  call load_module dir || "/rexx/format"
  call load_module dir || "/rexx/overlay"
  call load_module dir || "/rexx/sign"
  call load_module dir || "/rexx/space"
  call load_module dir || "/rexx/strip"
  call load_module dir || "/rexx/translate"
  call load_module dir || "/rexx/trunc"
  call load_module dir || "/rexx/upper"
  call load_module dir || "/rexx/verify"
  call load_module dir || "/rxas/word"
  call load_module dir || "/rxas/words"
  call load_module dir || "/rxas/wordindex"
  call load_module dir || "/rexx/reradix"
  call load_module dir || "/rexx/sequence"
  call load_module dir || "/rexx/find"
  call load_module dir || "/rexx/index"
  
  return 0


/* Poor mans datatype */
isint: procedure = .int
  arg expose val = .string
  len = length(val)
  if len = 0 then return 0
  do i = 1 to len
    c = substr(val,i,1)
    if c > '9' then return 0
    if c < '0' then return 0
  end
  return 1

/**************************/
/* REXX Library Functions */
/**************************/

/* abbrev(string,abbrebiated-string,min-char-match) */
abbrev: procedure = .string
  arg string = .string, astr = .string, len = 0

/* rexx center text  */
c2d: procedure = .int
  arg from = .string

c2x: procedure = .string
  arg from = .string

center: procedure = .string
  arg expose string = .string, centlen = .int,  pad = " "

centre: procedure = .string
  arg expose string = .string, centlen = .int,  pad = " "

/* changestr(needle,haystack,new-needle) returns string with replaced string */
changestr: procedure = .string
  arg expose needle = .string, expose haystack = .string, expose nneedle = .string /* Pass by reference */

/* abbrev(string,abbrebiated-string,min-char-match) */
compare: procedure = .string
  arg string = .string, astr = .string, pad = " "

/* countstr(needle,haystack) returns number occurrences of needle in haystack */
countstr: procedure = .int
  arg expose needle = .string, expose haystack = .string /* Pass by reference */

/* d2b( decimal to bit string)  returns bit combination of decimal string */
d2b: procedure = .string
   arg dec = .int

d2c: procedure = .string
  arg from = .int, slen=-1

/* d2x(hex-string)  returns hex string of integer  */
d2x: procedure = .string
  arg xint = .int, slen=-1

date: Procedure = .string
 arg oFormat = "", idate = "", iFormat = "", osep="", isep=""

/* delstr(string,position,length) delete string from certain position and length and returns it */
delstr: procedure = .string
  arg expose string = .string, position = .int, dellen = 0

/* delword(string,wordnumber-to-delete,length) delete one word, or the remaining words in string */
delword: procedure = .string
  arg expose string = .string, wnum = .int, wcount = -1

/*
 * crexx find
 * VM-TSO compatible implementation of wordpos()
 */
find: procedure = .int
  arg expose needle = .string, haystack = .string, start = 1

format: procedure = .string
   arg innum = .string, before = 0, after = 0, expp = 0, expt=-1

/* Raise() Internal Function to Raise a runtime error */
raise: procedure = .int
  arg type = .string, code = .string, parm1 = .string

/* Length() Procedure */
length: procedure = .int
  arg expose string1 = .string /* Pass by reference */

index: procedure = .int
  arg expose haystack = .string, needle = .string, start = 1

/* insert(insstr,string,position,length,pad) inserts string into existing string at certain position and length */
insert: procedure = .string
  arg insstr = .string, string = .string, position = .int, len = 0, pad = " "

/* rexx linesize bif */
/*
 * here mainly because it needs a native implementation on z/VM
 * and probably other OS; this one returns 999999999 to be consistent
 * with the Rexx compiler for zSeries
 */
linesize: procedure = .int
  arg expose string1 = .string

/* upper(string) translate to upper cases */
lower: procedure = .string
  arg expose string = .string

max: procedure = .float
  arg f1=.float,f2=.float,f3=-1e100,f4=-1e100,f5=-1e100,f6=-1e100,f7=-1e100,f8=-1e100,f9=-1e100,f10=-1e100,f11=-1e100,f12=-1e100,f13=-1e100,f14=-1e100,f15=-1e100,f16=-1e100

min: procedure = .float
  arg f1=.float,f2=.float,f3=1e100,f4=1e100,f5=1e100,f6=1e100,f7=1e100,f8=1e100,f9=1e100,f10=1e100,f11=1e100,f12=1e100,f13=1e100,f14=1e100,f15=1e100,f16=1e100

/* overlay(insstr,string,position,length,pad) overlays string into existing string at certain position and length */
overlay: procedure = .string
  arg insstr = .string, string = .string, position = .int, len = 0, pad = ""

/*
 *.  ReRadix
 *   Converts Arg(1) from radix Arg(2) to radix Arg(3)
 *   Radix range is 2-16.  Conversion is via decimal
 *   After Brian Marks' version
 */
reradix: procedure = .string
arg subject = .string, FromRadix = .int, ToRadix = .int

/* Built-in function Sequence is a modern day equivalent of XRANGE,
 * that can deal with Unicode. First turned up in NetRexx.
 * (XRANGE is limited to single byte characters 00-FF and wraps around)
 */
sequence: procedure = .string
arg from = .string, tos = .string

/* returns sign of number  */
sign: procedure = .int
  arg number = .float

/* rexx space adds n padding chars between words */

space: procedure = .string
  arg expose string = .string, spacenr = 1,  pad = " "

split: Procedure = .string
  arg idate = .string, isep=""

strip: procedure = .string
       arg instr = .string, option = "B", schar= " "

/* Substr() Procedure */
substr: procedure = .string
  arg string1 = .string, start = .int, len = length(string1) + 1 - start, pad = ' '

time: procedure = .string
   arg option = ""

/* translate  */
translate: procedure = .string
  arg source = .string, tochar = "?????", fromchar = "?????",pad=" "

trunc: procedure = .string
       arg innum = .string, fraction = 0

/* upper(string) translate to upper cases */
upper: procedure = .string
  arg expose string = .string

verify: procedure = .int
       arg instring = .string, intab = .string, match='N', spos=1

/* rexx wordlength finds length of certain word */
wordlength: procedure = .string
  arg expose string = .string, wordnum = .int

/* rexx wordpos searches for string and returns word position */
wordpos: procedure = .int
  arg expose search = .string, string = .string, start = 1

/* X2b(hex-string)  returns bit combination of hex string */
x2b: procedure = .string
 arg hex = .string, slen=-1

/* X2d(hex-string)  returns decimal number of hex string */
x2c: procedure = .string
  arg hex = .string

/* X2d(hex-string)  returns decimal number of hex string */
x2d: procedure = .int
  arg hex = .string, slen=-1

/*
 * XRANGE: deprecated
 * (XRANGE is limited to single byte characters 00-FF and wraps around)
 */
xrange: procedure = .string
arg from = .string, tos = .string

/**************************/
/* RXAS Library Functions */
/**************************/
word: procedure = .string
  arg string1 = .string, string2 = .int

words: procedure = .int
  arg string1 = .string

wordindex: procedure = .int
  arg string1 = .string, wordnum = .int

copies: procedure = .string
  arg string1 = .string, count = .int

pos: procedure = .int
  arg needle = .string, haystack = .string, offset = 1

right: procedure = .string
          arg string = .string, length1 = .int, pad = '0'/* rexx */

left: procedure = .string
  arg string1 = .string, len = .int, pad = '0'/* rexx */

_elapsed: procedure = .int
       arg string1 = .int

