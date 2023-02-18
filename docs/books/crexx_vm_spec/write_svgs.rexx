/*
 * rexx exec to produce svg files for the parameters
 */

/*
 digraph regs {
	node[shape=record]
	struct1 [label="IADD|REG|INT"];
     }
*/     

lineout('instruction_chapter.tex','% instruction chapter',1)
istem=''; istem=istem
istem[0]=1
outstem=''
istem[1]='select category, name from category order by category;'
address system 'sqlite3 ../../instructions/instructionbase.sqb' with -
  input stem istem -
  output stem outstem

do i=1 to outstem[0]
  parse outstem[i] cat'|'name
  /* now loop over the categories and pick out the corresponding instructions */
  istem=''; istem=istem
  istem[0]=1
  instructions=''
  istem[1]='select opcode, mnemonic, operands, description from instruction where category = 'cat' order by mnemonic;'
  address system 'sqlite3 ../../instructions/instructionbase.sqb' with -
    input stem istem -
    output stem instructions

  do j=1 to instructions[0]
    parse instructions[j] opcode '|' mnemonic '|' operands '|' description
    description=description.changestr('&','\\&')
    description=description.changestr('^','\\^')
    lineout(opcode'.gv','digraph regs {',1)
    lineout(opcode'.gv','node[shape=record]')
    line='struct1 [label='
    line=line'"'opcode'|'
    line=line''mnemonic.upper()'|'
    operands=operands.translate('','{')
    operands=operands.translate('','}')
    operands=operands.translate('|',',')
    line=line operands '"];'
    lineout(opcode'.gv',line)
    lineout(opcode'.gv','}')
    address system 'dot 'opcode'.gv -Tsvg -O'
  end -- do j
end -- do i
