/*
   Prototype CREXX Debugger
   Not complete but showing concepts around the introspection meta-instructions

   Version 0.1 - and that is being kind!
*/
options levelb
import rxfnsb
import globals

esc = '1B'x
green = esc"[32m"
reset = esc"[0m"
topleft = esc"[1;1H"
clear = esc"[2J"

say green"RXDB Version 0.1.2"
say ""
say "Loading CREXX Runtime Library Modules"
call load_library "../cmake-build-debug/lib/rxfns"

last_loaded_module = ""

/* REXX MODE */
mode = 'REXX'
call setmode mode

do forever
    cmd = ""
    say "Non-running state command (h=help) - Mode="mode":"
    assembler readline cmd

    c1 = word(cmd,1)
    c2 = word(cmd,2)

    if cmd = 'h' then do
       say 'help:'
       say '  q        - Quit'
       say '  m        - Toggle mode (REXX/ASM)'
       say '  e        - Print exposed procedures in module to be debugged (last module loaded)'
       say '  a        - Print exposed procedures (all modules)'
       say '  r proc   - Run procedure {proc}'
       say '  l rxbin  - Load rxbin module file {rxbin}'
    end
    else if cmd = 'q' then do
      leave
    end
    else if cmd = 'm' then do
       if mode = 'REXX' then mode = "ASM"
       else mode = 'REXX'
       call setmode mode
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
          say clear || topleft
          call setasm ""
          assembler bpon
          assembler dcall rc,call_id,no_args
          assembler bpoff
          say green"RXDB:" c2 "returned"
        end
      end
    end
    else say 'unknown command'
end

say "RXDB Exiting"reset
return 0

/* This is the interrupt handler that is called before every rxas instruction */
/* Note that interrupts are automatically disabled */
stephandler: procedure = .int
  arg expose address_object = .int;
  cmd = ""
  address = 0
  module = 0
  r = 0
  watch = ""
  assembler linkattr module,address_object,1  /* 1 = Module number */
  assembler linkattr address,address_object,2 /* 2 = Address in module */

  /* Are we the last module */
  modules = 0
  assembler metaloadedmodules modules
  if modules <> module then return 0 /* Don't debug the debugger */

  watch = getwatch()

  mode = getmode()
  if mode = "REXX" then do
    last_instruction = getrexx()
    next_instruction = ""
    rc = next_rexx(module, address, next_instruction)
    if rc = 0 then return 0 /* Not a rexx clause */
    call setrexx next_instruction
  end
  else do
    last_instruction = getasm()
    next_instruction = ""
    call next_asm module, address, next_instruction
    call setasm next_instruction
  end

  esc = '1B'x
  green = esc"[32m"
  reset = esc"[0m"
  topleft = esc"[1;1H"
  clear = esc"[2J"
  clearline = esc"[2K"
  cursorup = esc"[A"
  line2 = esc"[2;1H"
  line5 = esc"[5;1H"
  bottom = esc"[99;1H" || esc"[A"
  do forever
    say topleft || clearline || green || last_instruction;
    say line2 || clearline || green">"
    say clearline || next_instruction
    say clearline

    /* Print watches - can't move into a procedure [yet] because it needs to
       access the child's stackframe */
    do k = 2
      reg = word(watch,k)
      if reg = "" then leave
      value = ""
      ires = 0
      fres = 0.0
      sres = ""
      if isint(reg) then do
        /* Register Print */
        r = reg
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

        say clearline || "r" || r ":" value
      end

      else do
        /* REXX Variable Print */
        reg = lower(reg)
        symbol = ""
        type = ""
        meta_array = 0
        meta_entry = ""
        v = ""
        r_num = 0
        /* Read the addresses backwards but from the address before the code about to be executed */
        do a = address - 1 to 0 by -1
          /* Get the metadata for that address */
          assembler metaloaddata meta_array,module,a
          do i = 1 to meta_array
            assembler linkattr meta_entry,meta_array,i

            if meta_entry = ".meta_clear" then do /* Object type */
              assembler linkattr symbol,meta_entry,1
              if pos("."reg"@",symbol"@") > 0 then do /* TODO - Rough and ready find */
                leave a
              end
            end

            else if meta_entry = ".meta_const" then do /* Object type */
              assembler linkattr symbol,meta_entry,1
              if pos("."reg"@",symbol"@") > 0 then do /* TODO - Rough and ready find */
                assembler linkattr type,meta_entry,3
                assembler linkattr v,meta_entry,4
                value = "(CONSTANT" type")" v
                leave a
              end
            end

            else if meta_entry = ".meta_reg" then do /* Object type */
              assembler linkattr symbol,meta_entry,1
              if pos("."reg"@",symbol"@") > 0 then do /* TODO - Rough and ready find */
                assembler linkattr type,meta_entry,3
                assembler linkattr r_num,meta_entry,4

                if type = ".int" then do
                  assembler metalinkpreg ires,r_num       /* Link parent-frame-register */
                  ires_copy = ires /* Don't want to alter ires with any side effects */
                  assembler unlink ires
                  value = "(r"r_num ".int)" ires_copy
                end

                else if type = ".float" then do
                  assembler metalinkpreg fres,r_num       /* Link parent-frame-register */
                  fres_copy = fres
                  assembler unlink fres
                  value = "(r"r_num ".float)" fres_copy
                end

                else do
                  assembler metalinkpreg sres,r_num       /* Link parent-frame-register */
                  sres_copy = sres
                  assembler unlink sres
                  value = "(r"r_num ".STRING)" sres_copy
                end

              end
            end
          end
          assembler unlink symbol
          assembler unlink type
          assembler unlink meta_array
          assembler unlink meta_entry
          assembler unlink v
          assembler unlink r_num
        end
        if value = "" then value = "(TAKEN CONSTANT)" reg
        say clearline || reg ":" value
      end
    end

    say clearline
    say clearline"Running state command (ENTER=step, h=help) - Mode="mode":"
    say clearline || cursorup

    assembler readline cmd

    c1 = word(cmd,1)

    if cmd = '' then leave
    else if cmd = 'h' then do
       say clearline'help:'
       say clearline'  ENTER    - Step to next instruction'
       say clearline'  q        - Quit'
       say clearline'  w n ...  - Watch regs (numbers) or variables (names) ... (space delimited list of numbers and variable names)'
       say clearline'  e        - Print exposed procedures in module being debugged (last module loaded)'
       say clearline'  a        - Print exposed procedures (all modules)'
       say clearline'  m        - Toggle mode (REXX/ASM)'
    end
    else if cmd = 'q' then do
      say reset"Exiting"
      assembler exit
    end
    else if cmd = 'm' then do
      if mode = 'REXX' then mode = "ASM"
      else mode = 'REXX'
      call setmode mode
    end
    else if cmd = 'e' then do
      call dump_procs module
    end
    else if cmd = 'a' then do
      call dump_procs 0
    end
    else if c1 = 'w' then do
      watch = cmd;
      call setwatch watch
    end
    else say clearline'unknown command'
  end
  say reset || bottom
  return 0

/* This gets the rexx source code - returns 0 if the address does not start a rexx clause */
next_rexx: procedure = .int
  arg module = .int, address = .int, expose result  = .string

  rc = 0;
  result = ""

  meta_array = 0    /* Set to integer as the compiler doesn't understand arrays yet! */
  meta_entry = ""   /* Is an object really */
  line = 0;
  column = 0;
  source = "";

  /* Get the meta data for that address */
  assembler metaloaddata meta_array,module,address
  do i = 1 to meta_array
    assembler linkattr meta_entry,meta_array,i
    if meta_entry = ".meta_src" then do /* Object type */
      rc = 1; /* We are at the start of a clause */
      assembler linkattr line,meta_entry,1
      assembler linkattr column,meta_entry,2
      assembler linkattr source,meta_entry,3
      result = result || "(" || line || ":" || column || ") '" || source || "'; "
    end
  end

  return rc

/* This disassembles the code at address */
next_asm: procedure = .int
  arg module = .int, address = .int, expose result  = .string
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
  result = " " right(d2x(opcode),3,"0") "@" right(d2x(module),3,"0")":"right(d2x(address),4,"0") instruction,
          opdesc(op1_type,module,address+1)","opdesc(op2_type,module,address+2)","opdesc(op3_type,module,address+3),
           "*" description

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
  return 0


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
  call load_module dir || "/library"
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


