/* REXX */
/* -------------------------------------------------------------------------------
 * Generate Instruction set statements
 * -------------------------------------------------------------------------------
 */
  parse arg path

  file=path"/machine/rxvminst.c"
  ofile=path"/machine/instrset.h"

  asm=path"/interpreter/rxvmintp.c"
  asmmiss=path"/machine/instrmiss.h"

  call lineout ofile,,1 /* truncation */
  call lineout asmmiss,,1 /* truncation */

  threaded_inst.0 = 0
  bytecode_inst.0 = 0
  meta_inst.0 = 0

  call inc_inst '/* -------------------------------------------------------------------------------'
  call inc_inst ' * Generate Instruction Set, generated on 'date()' AT 'time()
  call inc_inst ' * -------------------------------------------------------------------------------'
  call inc_inst ' */'
  call inc_inst '#include "rxvminst.h"'

  call fetchLabel   /* Analyse defined labels in rx_intrp.c */

/* find first instruction definition in operands.c */
  do until lines(file)=0 | pos('void init_ops()',linein(file))>0
  end

  call add_meta_inst     'Instruction meta_map[] = {   {0,"null","",0,OP_NONE,OP_NONE,OP_NONE},'
  call add_threaded_inst 'void *address_map[] = {  &&INULL,'
  call add_bytecode_inst 'enum instructions {      INST_INULL,'

/* run through all instruction definitions in operands.c */
  lino=0
  not=0
  inst=0
  do while lines(file)>0
     line=strip(linein(file))
     if line='' then iterate
     if pos('instr_f(',line)=0 then leave  /* no more instructions end loop */
     lino=lino+1
     interpret 'rc='line   /* execute instr_f function via a REXX function call */
  end

  inst=inst+1
  call add_meta_inst     '                             {'inst',"breakpoint","",0,OP_NONE,OP_NONE,OP_NONE},'
  call add_threaded_inst '                         &&BREAKPOINT,'
  call add_bytecode_inst '                         INST_BREAKPOINT,'

  inst=inst+1
  call add_meta_inst     '                             {'inst',"unknown","",0,OP_NONE,OP_NONE,OP_NONE} };'
  call add_threaded_inst '                         &&IUNKNOWN };'
  call add_bytecode_inst '                         INST_IUNKNOWN };'

  call inc_inst ""
  do i = 1 to meta_inst.0
    call inc_inst meta_inst.i
  end

  call inc_inst ""
  call inc_inst "#ifdef NTHREADED"
  call inc_inst ""

  do i = 1 to bytecode_inst.0
    call inc_inst bytecode_inst.i
  end

  call inc_inst ""
  call inc_inst "#else"
  call inc_inst ""

  do i = 1 to threaded_inst.0
    call inc_inst threaded_inst.i
  end

  call inc_inst ""
  call inc_inst "#endif"

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
/* next instruction */
inst=inst+1

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

  /* generate instruction */
  call add_meta_inst     '                             {'inst',"'cmd'","'txt'",'numparm','r1','r2','r3'},'
  call add_threaded_inst '                         &&'ucmd','
  call add_bytecode_inst '                         INST_'ucmd','

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
	call inc_miss 'START_INSTRUCTION('ucmd') // label not yet defined'
	call inc_miss '  CALC_DISPATCH('numparm');'
    call inc_miss '    DEBUG("TRACE - 'ucmd'\n");'
    call inc_miss '    DEBUG("'ucmd' not yet defined\n");'
	call inc_miss '    goto SIGNAL;'

/*
	do ni=1 to numparm
	   call addCode ni
	end
*/
	call inc_miss '    // Add your coding '
    call inc_miss '    /* REG_OP(1)="?????"; */'

 	call inc_miss '  DISPATCH;'
    call inc_miss '  '
  end
return 

addCode:
  parse arg cnum
    if word(instr,cnum+1)='REG' then do
        call inc_miss '    v'cnum' = REG_OP('cnum');'
        call inc_miss '    REG_TEST(!v'cnum');' 
	end
	else if word(instr,cnum+1)='INT' then do
       call inc_miss '    i'cnum' = INT_OP('cnum');'
	end
	else if word(instr,cnum+1)='FLOAT' then do
       call inc_miss '    f'cnum' = FLOAT_OP('cnum');'
	end
	else if word(instr,cnum+1)='STRING' then do
       call inc_miss '    s'cnum' = CONSTSTRING_OP('cnum');'
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
     if pos('START_INSTRUCTION(',line)=0 then iterate
     label=word(line,1)
     parse value label with 'START_INSTRUCTION(' label ')' .
     li=li+1
     label.li=translate(label)
  end
  label.0=li
return

/* -------------------------------------------------------------------------------
 * Buffer Instruction (Threaded)
 * -------------------------------------------------------------------------------
 */
add_threaded_inst: procedure expose threaded_inst.
  parse arg line
  i = threaded_inst.0 + 1
  threaded_inst.0 = i
  threaded_inst.i = line
return

/* -------------------------------------------------------------------------------
 * Buffer Instruction (Bytecode)
 * -------------------------------------------------------------------------------
 */
add_bytecode_inst: procedure expose bytecode_inst.
  parse arg line
  i = bytecode_inst.0 + 1
  bytecode_inst.0 = i
  bytecode_inst.i = line
return

/* -------------------------------------------------------------------------------
 * Buffer Instruction (Metadata)
 * -------------------------------------------------------------------------------
 */
add_meta_inst: procedure expose meta_inst.
  parse arg line
  i = meta_inst.0 + 1
  meta_inst.0 = i
  meta_inst.i = line
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