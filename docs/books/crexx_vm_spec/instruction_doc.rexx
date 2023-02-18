/* rexx exec to produce the part of the vm specification that deals with rxvm instructions
 * uses an sqlite database wherein the instructions are categorized for the purpose
 * of grouping them in the documentation.
 *
 * we select these in the sequence of the 'category'
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

  lineout('instruction_chapter.tex','\\section{'name'}')
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
    lineout('instruction_chapter.tex','\\begin{description}')
    lineout('instruction_chapter.tex','\\item[\\texttt{'mnemonic.upper'}] 'description'\\\\')
    lineout('instruction_chapter.tex','\\end{description}')
    lineout('instruction_chapter.tex','\\includesvg{svg/'opcode'.gv}')
  end -- do j
end -- do i
