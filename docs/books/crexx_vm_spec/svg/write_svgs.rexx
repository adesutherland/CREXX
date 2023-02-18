/*
 * rexx exec to produce svg files for the parameters
 */
lineout('instruction_chapter.tex','% instruction chapter',1)
istem=''; istem=istem
istem[0]=1
outstem=''
istem[1]='select category, name from category order by category;'
address system 'sqlite3 ../../../instructions/instructionbase.sqb' with -
  input stem istem -
  output stem outstem

do i=1 to outstem[0]
  parse outstem[i] cat'|'name
  /* now loop over the categories and pick out the corresponding instructions */
  istem=''; istem=istem
  istem[0]=1
  instructions=''
  istem[1]='select opcode, mnemonic, operands, description from instruction where category = 'cat' order by mnemonic;'
  address system 'sqlite3 ../../../instructions/instructionbase.sqb' with -
    input stem istem -
    output stem instructions

/* example:
digraph {
  graph [pad="0.5", nodesep="0.5", ranksep="2" ]
  node  [shape=plain]
Foo [label=<
<table border="0" cellborder="1" cellspacing="0">
  <tr>  <td> Opcode </td><td> Name </td><td> OP1 </td> <td> OP2 </td> <td> OP3 </td>  </tr>
  <tr>  <td> 0x0107</td><td> ADDI </td><td> INT </td><td> REG </td><td> REG </td></tr>
</table>>];
} 
*/     
  
  do j=1 to instructions[0]
    parse instructions[j] opcode '|' mnemonic '|' operands '|' description
    description=description.changestr('&','\\&')
    description=description.changestr('^','\\^')
    lineout(opcode'.gv','digraph{',1)
    lineout(opcode'.gv','graph [pad="0.5", nodesep="0.5", ranksep="2" ]')
    lineout(opcode'.gv','node[shape=plain]')
    lineout(opcode'.gv','Foo [label=<')
    lineout(opcode'.gv','<table border="0" cellborder="1" cellspacing="0">')
    line='<tr><td> Opcode </td> <td> Name </td> '
    /* here the opc1 opc2 etc labels */
    placeholder=operands --make a copy
    opclb=0
    do until placeholder=''
      opclb=opclb+1
      parse placeholder first ',' placeholder
      line=line'<td> OP'opclb '</td>'
    end
    line=line '</tr>'
    lineout(opcode'.gv',line)
    /* now the line with the operand types */
    operands=operands.translate('  ','{}')
    line="<tr>"
    line=line'"<td>' opcode.strip() '</td>'
    line=line'<td>' mnemonic.upper().strip() '</td>'
    do until operands=''
      opclb=opclb+1
      parse operands oper ',' operands
      line=line'<td>' oper.strip() '</td>'
    end
    line=line '</tr>'
    lineout(opcode'.gv',line)
    lineout(opcode'.gv','</table>>];')
    lineout(opcode'.gv','}')
    address system 'dot 'opcode'.gv -Tsvg -O'
  end -- do j
end -- do i
