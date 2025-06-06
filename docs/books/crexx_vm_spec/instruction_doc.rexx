/* rexx exec to produce the part of the vm specification that deals with rxvm instructions
 * uses an sqlite database wherein the instructions are categorized for the purpose
 * of grouping them in the documentation.
 *
 * we select these in the sequence of the 'category'
 * instructions are grouped by mnemonic and have a table for all opcodes (wip) 
 */

lineout('instruction_chapter.tex','% instruction chapter',1)
/*
 * select the full names for the instructions
 */
instructionNames=''
ststem=''; ststem=ststem
ststem[0]=1
stemout=''
ststem[1]='select mnemonic, description from inst_name;'
address system 'sqlite3 ../../instructions/instructionbase.sqb' with -
  input stem ststem -
  output stem stemout
do o=1 to stemout[0]
  parse stemout[o] mnem '|' name
  instructionNames[mnem]=name
end


decopt = '-p "$CREXX_HOME/interpreter/rxvmplugin/rxvmplugins/mc_decimal/rxvm_mc_decimal"'
/*
 * select the categories
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
  istem[1]='select distinct mnemonic from inst_cat where category = 'cat' order by mnemonic;'
  address system 'sqlite3 ../../instructions/instructionbase.sqb' with -
    input stem istem -
    output stem instructions

  do j=1 to instructions[0]
    parse instructions[j] mnemonic 
    description = ' -' instructionNames[mnemonic]
    if description = '' then description='instruction'

    /* get the opcodes for this instruction mnemonic */
    jstem=''; jstem=jstem
    jstem[0]=1
    ops=''
    jstem[1]="select opcode, mnemonic, operands, description from instruction where mnemonic = '"mnemonic"' order by opcode;"
    address system 'sqlite3 ../../instructions/instructionbase.sqb' with -
      input stem jstem -
      output stem ops
    if pos('eprecated',ops[1]) >0 then iterate -- we do not include deprecated instructions

    lineout('instruction_chapter.tex','\\begin{description}')
    lineout('instruction_chapter.tex','\\item[\\texttt{'mnemonic.upper'}] 'description'\\index{'mnemonic.lower()'}\\\\')

    if ops[0]==1 then
      do
	lineout('instruction_chapter.tex','\\item[\\texttt{Syntax}] - form \\\\')
      end
    else do
      lineout('instruction_chapter.tex','\\item[\\texttt{Syntax}] - variants\\\\')
    end

    lineout('instruction_chapter.tex','\\fontspec{IBM Plex Mono}')
    lineout('instruction_chapter.tex','\\includesvg{../../svg/'mnemonic'.gv}')
    lineout('instruction_chapter.tex','\\fontspec{TeX Gyre Pagella}')
    lineout('instruction_chapter.tex','\\item[\\texttt{Operation}]')
    lineout('instruction_chapter.tex','\\IfFileExists{operation/'mnemonic'.operation}{\\input{operation/'mnemonic'.operation}}{}')
    lineout('instruction_chapter.tex','\\item[\\texttt{}]')
    do o=1 to ops[0]
      parse ops[o] opcode '|' mn '|' oper '|' descriptor
      descriptor=descriptor.changestr('&','\\&')
      descriptor=descriptor.changestr('^','\\^')
      parse oper oper .
      oper=oper.translate('',',')
      oper=oper.translate('','}')
      oper=oper.translate('','{')
      oper=oper.strip().space(0)
      lineout('instruction_chapter.tex','\\item[\\texttt{'opcode'}]\\fontspec{IBM Plex Sans Condensed}'descriptor oper'\\\\')
      lineout('instruction_chapter.tex','\\fontspec{TeX Gyre Pagella}')
      lineout('instruction_chapter.tex','\\IfFileExists{../../examples/'mnemonic||oper'.def}{\\input{../../examples/'mnemonic||oper'.def}}{}')
      lineout('instruction_chapter.tex','\\IfFileExists{../../examples/'mnemonic||oper'.rexx}{\\splice{rxc ../../examples/'mnemonic||oper'}}{}')
      lineout('instruction_chapter.tex','\\IfFileExists{../../examples/'mnemonic||oper'.rxas}{\\lstinputlisting[language=rxas,label='mn',caption='mn' example.]{../../examples/'mnemonic||oper'.rxas} \\splice{rxas ../../examples/'mnemonic||oper'} \\obeylines \\begin{shaded} \\fontspec{IBM Plex Mono}[Scale=0.8] \\splice{rxvme 'decopt' ../../examples/'mnemonic||oper'} \\end{shaded} \\fontspec{TeX Gyre Pagella}}{}')
    end
    lineout('instruction_chapter.tex','\\end{description}')
    lineout('instruction_chapter.tex','\\clearpage')
  end -- do j
end -- do i
