/* REXX */
/* -------------------------------------------------------------------------------
 * Generate Instruction set statements
 * -------------------------------------------------------------------------------
 */
path="C:\Users\PeterJ\CLionProjects\CREXX\assembler\"
file=path"operands.c"
ofile=path"instrset.h"

  asm=path"rx_intrp.c"
  asmmiss=path"instrmiss.h"

  ADDRESS SYSTEM 'DEL 'ofile
  ADDRESS SYSTEM 'DEL 'asmmiss

  call inc_inst,'/* -------------------------------------------------------------------------------'
  call inc_inst,' * Generate Instruction Set, generated on 'date()' AT 'time()
  call inc_inst,' * -------------------------------------------------------------------------------'
  call inc_inst,' */'

  call fetchLabel   /* Analyse defined labels in rx_intrp.c */

/* find first instruction definition in operands.c */
  do until lines(file)=0 | pos('void init_ops()',linein(file))>0
  end
/* run through all instruction definitions in operands.c */
  lino=0
  not=0
  do while lines(file)>0
     line=strip(linein(file))
     if line='' then iterate
     if pos('instr_f(',line)=0 then leave  /* no more instructions end loop */
     lino=lino+1
     interpret 'rc='line   /* execute instr_f function via a REXX function call */
end
say lino-not' functions are defined'
say not' functions are not yet defined'
say lino' functions in total'

exit 0
/* -------------------------------------------------------------------------------
 * Generate Instruction Clause
 * -------------------------------------------------------------------------------
 */
instr_f:
  parse arg cmd,txt,label1,label2,label3
/* set parameter for src_inst instruction, should not happen, maybe always set */
  if label1='' then r1='OP_NONE'
     else r1=label1
  if label2='' then r2='OP_NONE'
     else r2=label2
  if label3='' then r3='OP_NONE'
     else r3=label3

/* create label suffix  */
  if r1='OP_NONE' then label1=''
     else parse value r1 with 'OP_'label1
  if r2='OP_NONE' then label2=''
     else parse value r2 with 'OP_'label2
  if r3='OP_NONE' then label3=''
     else parse value r3 with 'OP_'label3

  ucmd=translate(cmd)
/* generate instruction */
  call inc_inst '// ----- 'cmd' Instruction 'txt' -----'
  call inc_inst 'instruction = src_inst("'cmd'", 'r1','r2','r3');'
  if      label1='' then numparm=0
  else if label2='' then do
     ucmd=ucmd'_'label1
	 numparm=1
  end
  else if label3='' then do
     ucmd=ucmd'_'label1'_'label2
     numparm=2
  end
  else do
     ucmd=ucmd'_'label1'_'label2'_'label3
     numparm=3
  end

  call inc_inst 'if (instruction) address_map[instruction->opcode] = &&'ucmd';'
  call inc_inst '    else print_debug("Instruction 'ucmd' not found\n");'
  call inc_inst '  '

  call alreadyDefined       /* cross check if label is defined or missing */
return 0
/* -------------------------------------------------------------------------------
 * Test if instruction is already defined, else write into not yet defined include
 * -------------------------------------------------------------------------------
 */
alreadyDefined:
  found=0
 
  do l=1 to label.0
     if ucmd=label.l then do
        found=1
        leave
     end
  end
  instr=translate(ucmd,,'_')

  if found>0 then nop /*call inc_miss ,'// > 'ucmd': // label already defined' */
  else do
    not=not+1
	call inc_miss '/* ------------------------------------------------------------------------------------'
    call inc_miss ' *  'ucmd'  'txt'              pej 'date()
    call inc_miss ' *  -----------------------------------------------------------------------------------'
    call inc_miss ' */'
	call inc_miss ucmd': // label not yet defined'
	call inc_miss '  CALC_DISPATCH('numparm');'
    call inc_miss '    print_debug("'ucmd' not yet defined\n");'
	call inc_miss '    goto SIGNAL;'

	do ni=1 to numparm
	   call addCode ni
	end

	call inc_miss '    // Add your coding '
    call inc_miss '    REG_OP(1)="?????";'

 	call inc_miss '  DISPATCH;'
    call inc_miss '  '
  end
return 

addCode:
  parse arg cnum
    if word(instr,cnum+1)='REG' then do
        call inc_miss '    v'cnum' = REG_OP('cnum');'
        call inc_miss '    if (!v'cnum') REG_NOT();'
	end
	else if word(instr,cnum+1)='INT' then do
       call inc_miss '    i'cnum' = INT_OP('cnum');'
	end
	else if word(instr,cnum+1)='FLOAT' then do
       call inc_miss '    f'cnum' = FLOAT_OP('cnum');'
	end
	else if word(instr,cnum+1)='STRING' then do
       call inc_miss '    s'cnum' = ONSTSTRING_OP('cnum');'
	end
return

/* -------------------------------------------------------------------------------
 * Check existing labels in rx_intrp.c
 * -------------------------------------------------------------------------------
 */
fetchLabeL:
  li=0
  do while lines(asm)>0
     line=linein(asm)
     if pos(':',line)=0 then iterate
     label=word(line,1)
     parse value label with label':'remain
     li=li+1
     label.li=translate(label)
  end
  label.0=li
return
/* -------------------------------------------------------------------------------
 * Write to Instruction include file
 * -------------------------------------------------------------------------------
 */
inc_inst:
  call lineout ofile,arg(1)
return
/* -------------------------------------------------------------------------------
 * Write to Instruction include file
 * -------------------------------------------------------------------------------
 */
inc_miss:
  call lineout asmmiss,arg(1)
return