/* rexx exec to produce the part of the vm specification that deals with rxvm instructions
 * uses an sqlite database wherein the instructions are categorized for the purpose
 * of grouping them in the documentation.
 *
 * we select these in the sequence of the 'category'
 * instructions are grouped by mnemonic and have a table for all opcodes (wip) 
 */

lineout('instruction_chapter.tex','% instruction chapter',1)
/*
 * first select the categories
 */  
istem=''; istem=istem
istem[0]=1
outstem=''
istem[1]='select category, name from category order by category;'
address system 'sqlite3 ../../instructions/instructionbase.sqb' with -
  input stem istem -
  output stem outstem

do i=1 to outstem[0]
  /* for categories, we now have cat (an integer) and name (descriptor) */
  parse outstem[i] cat'|'name

  lineout('instruction_chapter.tex','\\section{'name'}')
  /* now loop over the categories and pick out the corresponding instructions */
  istem=''; istem=istem
  istem[0]=1
  instructions=''
  istem[1]='select distinct mnemonic from instruction where category = 'cat' order by mnemonic;'
  address system 'sqlite3 ../../instructions/instructionbase.sqb' with -
    input stem istem -
    output stem instructions

  do j=1 to instructions[0]
    parse instructions[j] mnemonic 
    description = 'instruction'
    lineout('instruction_chapter.tex','\\begin{description}')
    lineout('instruction_chapter.tex','\\item[\\texttt{'mnemonic.upper'}] 'description'\\\\')
    /* get the opcodes for this instruction mnemonic */
    jstem=''; jstem=jstem
    jstem[0]=1
    ops=''
    jstem[1]="select opcode, mnemonic, operands, description from instruction where mnemonic = '"mnemonic"' order by opcode;"
    address system 'sqlite3 ../../instructions/instructionbase.sqb' with -
      input stem jstem -
      output stem ops
    if ops[0]==1 then
      do
	lineout('instruction_chapter.tex','\\item[\\texttt{Syntax}] - form \\\\')
      end
    else do
      lineout('instruction_chapter.tex','\\item[\\texttt{Syntax}] - variants\\\\')
    end

    lineout('instruction_chapter.tex','\\fontspec{IBM Plex Mono}')
    lineout('instruction_chapter.tex','\\includesvg{svg/'mnemonic'.gv}')
    lineout('instruction_chapter.tex','\\fontspec{TeX Gyre Pagella}')
    lineout('instruction_chapter.tex','\\item[\\texttt{Operation}]')
    lineout('instruction_chapter.tex','\\IfFileExists{examples/'mnemonic'.operation}{\\input{examples/'mnemonic'.operation}}{}')
    lineout('instruction_chapter.tex','\\item[\\texttt{}]')
    do o=1 to ops[0]
      parse ops[o] opcode '|' mn '|' oper '|' descriptor
      descriptor=descriptor.changestr('&','\\&')
      descriptor=descriptor.changestr('^','\\^')
      lineout('instruction_chapter.tex','\\item[\\texttt{'opcode'}]\\fontspec{IBM Plex Sans Condensed}'descriptor oper'\\\\')
      lineout('instruction_chapter.tex','\\fontspec{TeX Gyre Pagella}')
      lineout('instruction_chapter.tex','\\IfFileExists{examples/'opcode'.def}{\\input{examples/'opcode'.def}}{}')
      lineout('instruction_chapter.tex','\\IfFileExists{examples/'opcode'.rxas}{\\lstinputlisting[language=rxas,label='mn',caption='mn' example.]{examples/'opcode'.rxas} \\splice{rxas examples/'opcode'} \\obeylines \\splice{rxvm examples/'opcode'}}{}')
    end
    lineout('instruction_chapter.tex','\\end{description}')
    lineout('instruction_chapter.tex','\\clearpage')
  end -- do j
end -- do i
