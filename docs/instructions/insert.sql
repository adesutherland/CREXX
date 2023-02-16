insert into instruction VALUES('0x0052','appendchar ',' {REG,REG}           ',' Append Concat Char op2 (as int) on op1','9');
insert into instruction VALUES('0x0027','addf       ',' {REG,REG,FLOAT}     ',' Convert and Add to Float (op1=op2+op3) (Deprecated)','2');
insert into instruction VALUES('0x0011','addi       ',' {REG,REG,REG}       ',' Convert and Add to Integer (op1=op2+op3) (Deprecated)','1');
insert into instruction VALUES('0x0012','addi       ',' {REG,REG,INT}       ',' Convert and Add to Integer (op1=op2+op3) (Deprecated)','1');
insert into instruction VALUES('0x009e','amap       ',' {REG,REG}           ',' Map op1 to arg register index in op2','7');
insert into instruction VALUES('0x009f','amap       ',' {REG,INT}           ',' Map op1 to arg register index  op2','7');
insert into instruction VALUES('0x0096','and        ',' {REG,REG,REG}       ',' Logical (int) and op1=(op2 && op3)','3');
insert into instruction VALUES('0x0055','append     ',' {REG,REG}           ',' String Append (op1=op1||op2)','9');
insert into instruction VALUES('0x0026','addf       ',' {REG,REG,REG}       ',' Convert and Add to Float (op1=op2+op3) (Deprecated)','2');
insert into instruction VALUES('0x00eb','bcf        ',' {ID,REG}            ',' if op2=0 goto op1(if false) else dec op2','4');
insert into instruction VALUES('0x00ec','bcf        ',' {ID,REG,REG}        ',' if op2=0 goto op1(if false) else dec op2 and inc op3','4');
insert into instruction VALUES('0x00e7','bct        ',' {ID,REG}            ',' dec op2; if op2>0; goto op1(if true)','4');
insert into instruction VALUES('0x00e8','bct        ',' {ID,REG,REG}        ',' dec op2; inc op3, if op2>0; goto op1(if true)','4');
insert into instruction VALUES('0x00e9','bctnm      ',' {ID,REG}            ',' dec op2; if op2>=0; goto op1(if true)','4');
insert into instruction VALUES('0x00ea','bctnm      ',' {ID,REG,REG}        ',' dec op2; inc op3, if op2>=0; goto op1(if true)','4');
insert into instruction VALUES('0x00f7','beq        ',' {ID,REG,REG}        ',' if op2==op3 then goto op1','4');
insert into instruction VALUES('0x00ef','bge        ',' {ID,REG,REG}        ',' if op2>=op3 then goto op1','4');
insert into instruction VALUES('0x00f0','bge        ',' {ID,REG,INT}        ',' if op2>=op3 then goto op1','4');
insert into instruction VALUES('0x00ed','bgt        ',' {ID,REG,REG}        ',' if op2>op3 then goto op1','4');
insert into instruction VALUES('0x00ee','bgt        ',' {ID,REG,INT}        ',' if op2>op3 then goto op1','4');
insert into instruction VALUES('0x00f3','ble        ',' {ID,REG,REG}        ',' if op2<=op3 then goto op1','4');
insert into instruction VALUES('0x00f4','ble        ',' {ID,REG,INT}        ',' if op2<=op3 then goto op1','4');
insert into instruction VALUES('0x00f1','blt        ',' {ID,REG,REG}        ',' if op2<op3 then goto op1','4');
insert into instruction VALUES('0x00f2','blt        ',' {ID,REG,INT}        ',' if op2<op3 then goto op1','4');
insert into instruction VALUES('0x00f5','bne        ',' {ID,REG,REG}        ',' if op2!=op3 then goto op1','4');
insert into instruction VALUES('0x00f6','bne        ',' {ID,REG,INT}        ',' if op2!=op3 then goto op1','4');
insert into instruction VALUES('0x0002','bpoff      ',' no operand          ',' Disable Breakpoints','8');
insert into instruction VALUES('0x0001','bpon       ',' no operand          ',' Enable Breakpoints','8');
insert into instruction VALUES('0x00b3','br         ',' {ID}                ',' Branch to op1','4');
insert into instruction VALUES('0x00b5','brf        ',' {ID,REG}            ',' Branch to op1 if op2 false','4');
insert into instruction VALUES('0x00b4','brt        ',' {ID,REG}            ',' Branch to op1 if op2 true','4');
insert into instruction VALUES('0x0104','brtpandt   ',' {ID,REG,INT}        ',' if op2.typeflag && op3 true then goto op1','4');
insert into instruction VALUES('0x00f8','beq        ',' {ID,REG,INT}        ',' if op2==op3 then goto op1','4');
insert into instruction VALUES('0x0103','brtpt      ',' {ID,REG}            ',' if op2.typeflag true then goto op1','4');
insert into instruction VALUES('0x00b6','brtf       ',' {ID,ID,REG}         ',' Branch to op1 if op3 true, otherwise branch to op2','4');
insert into instruction VALUES('0x00ab','call       ',' {REG,FUNC,REG}      ',' Call procedure (op1=op2(op3...) )','4');
insert into instruction VALUES('0x00e1','cnop       ',' no operand          ',' no operation','4');
insert into instruction VALUES('0x00a9','call       ',' {FUNC}              ',' Call procedure (op1())','4');
insert into instruction VALUES('0x0053','concchar   ',' {REG,REG,REG}       ',' Concat Char op1 from op2 position op3','9');
insert into instruction VALUES('0x004f','concat     ',' {REG,REG,REG}       ',' String Concat (op1=op2||op3)','9');
insert into instruction VALUES('0x0050','concat     ',' {REG,REG,STRING}    ',' String Concat (op1=op2||op3)','9');
insert into instruction VALUES('0x0051','concat     ',' {REG,STRING,REG}    ',' String Concat (op1=op2||op3)','9');
insert into instruction VALUES('0x00aa','call       ',' {REG,FUNC}          ',' Call procedure (op1=op2())','4');
insert into instruction VALUES('0x00b9','copy       ',' {REG,REG}           ',' Copy op2 to op1','3');
insert into instruction VALUES('0x0037','divf       ',' {REG,FLOAT,REG}     ',' Convert and Divide to Float (op1=op2/op3) (Deprecated)','2');
insert into instruction VALUES('0x0039','dec        ',' {REG}               ',' Decrement Int (op1--)','1');
insert into instruction VALUES('0x003b','dec0       ',' no operand          ',' Decrement R0-- Int','1');
insert into instruction VALUES('0x00ac','dcall      ',' {REG,REG,REG}       ',' Dynamic call procedure (op1=op2(op3...) )','3');
insert into instruction VALUES('0x003f','dec2       ',' no operand          ',' Decrement R2-- Int','1');
insert into instruction VALUES('0x003d','dec1       ',' no operand          ',' Decrement R1-- Int','1');
insert into instruction VALUES('0x0035','divf       ',' {REG,REG,REG}       ',' Convert and Divide to Float (op1=op2/op3) (Deprecated)','2');
insert into instruction VALUES('0x001f','divi       ',' {REG,REG,REG}       ',' Convert and Divide to Integer (op1=op2/op3) (Deprecated)','1');
insert into instruction VALUES('0x0020','divi       ',' {REG,REG,INT}       ',' Convert and Divide to Integer (op1=op2/op3) (Deprecated)','1');
insert into instruction VALUES('0x0106','dllparms   ',' {REG,REG,REG}       ',' fetches parms for DLL call ','3');
insert into instruction VALUES('0x00dc','dropchar   ',' {REG,REG,REG}       ',' set op1 from op2 after dropping all chars from op3','9');
insert into instruction VALUES('0x0036','divf       ',' {REG,REG,FLOAT}     ',' Convert and Divide to Float (op1=op2/op3) (Deprecated)','2');
insert into instruction VALUES('0x00ce','exit       ',' no operand          ',' Exit','3');
insert into instruction VALUES('0x00cf','exit       ',' {REG}               ',' Exit op1','3');
insert into instruction VALUES('0x00d0','exit       ',' {INT}               ',' Exit op1','3');
insert into instruction VALUES('0x0024','fadd       ',' {REG,REG,REG}       ',' Float Add (op1=op2+op3)','2');
insert into instruction VALUES('0x0025','fadd       ',' {REG,REG,FLOAT}     ',' Float Add (op1=op2+op3)','2');
insert into instruction VALUES('0x00bb','fcopy      ',' {REG,REG}           ',' Copy Float op2 to op1','2');
insert into instruction VALUES('0x0032','fdiv       ',' {REG,REG,REG}       ',' Float Divide (op1=op2/op3)','2');
insert into instruction VALUES('0x0033','fdiv       ',' {REG,REG,FLOAT}     ',' Float Divide (op1=op2/op3)','2');
insert into instruction VALUES('0x0034','fdiv       ',' {REG,FLOAT,REG}     ',' Float Divide (op1=op2/op3)','2');
insert into instruction VALUES('0x0075','feq        ',' {REG,REG,FLOAT}     ',' Float Equals op1=(op2==op3)','2');
insert into instruction VALUES('0x00d8','fformat    ',' {REG,REG,REG}       ',' Set string value from float value using a format string','2');
insert into instruction VALUES('0x0078','fgt        ',' {REG,REG,REG}       ',' Float Greater than op1=(op2>op3)','2');
insert into instruction VALUES('0x0079','fgt        ',' {REG,REG,FLOAT}     ',' Float Greater than op1=(op2>op3)','2');
insert into instruction VALUES('0x007a','fgt        ',' {REG,FLOAT,REG}     ',' Float Greater than op1=(op2>op3)','2');
insert into instruction VALUES('0x007b','fgte       ',' {REG,REG,REG}       ',' Float Greater than equals op1=(op2>=op3)','2');
insert into instruction VALUES('0x007c','fgte       ',' {REG,REG,FLOAT}     ',' Float Greater than equals op1=(op2>=op3)','2');
insert into instruction VALUES('0x007d','fgte       ',' {REG,FLOAT,REG}     ',' Float Greater than equals op1=(op2>=op3)','2');
insert into instruction VALUES('0x007e','flt        ',' {REG,REG,REG}       ',' Float Less than op1=(op2<op3)','2');
insert into instruction VALUES('0x007f','flt        ',' {REG,REG,FLOAT}     ',' Float Less than op1=(op2<op3)','2');
insert into instruction VALUES('0x0080','flt        ',' {REG,FLOAT,REG}     ',' Float Less than op1=(op2<op3)','2');
insert into instruction VALUES('0x0081','flte       ',' {REG,REG,REG}       ',' Float Less than equals op1=(op2<=op3)','2');
insert into instruction VALUES('0x0082','flte       ',' {REG,REG,FLOAT}     ',' Float Less than equals op1=(op2<=op3)','2');
insert into instruction VALUES('0x0083','flte       ',' {REG,FLOAT,REG}     ',' Float Less than equals op1=(op2<=op3)','2');
insert into instruction VALUES('0x002e','fmult      ',' {REG,REG,REG}       ',' Float Multiply (op1=op2*op3)','2');
insert into instruction VALUES('0x002f','fmult      ',' {REG,REG,FLOAT}     ',' Float Multiply (op1=op2*op3)','2');
insert into instruction VALUES('0x00f9','fndblnk    ',' {REG,REG,REG}       ',' op1 = find next blank in op2[op3] and behind','9');
insert into instruction VALUES('0x00fa','fndnblnk   ',' {REG,REG,REG}       ',' op1 = find next next non blank in op2[op3] and behind','9');
insert into instruction VALUES('0x0076','fne        ',' {REG,REG,REG}       ',' Float Not equals op1=(op2!=op3)','2');
insert into instruction VALUES('0x0077','fne        ',' {REG,REG,FLOAT}     ',' Float Not equals op1=(op2!=op3)','2');
insert into instruction VALUES('0x00e5','fpow       ',' {REG,REG,REG}       ',' op1=op2**op3','2');
insert into instruction VALUES('0x00e6','fpow       ',' {REG,REG,FLOAT}     ',' op1=op2**op3','2');
insert into instruction VALUES('0x00fc','fsex       ',' {REG}               ',' float op1 = -op1 (sign change)','2');
insert into instruction VALUES('0x0028','fsub       ',' {REG,REG,REG}       ',' Float Subtract (op1=op2-op3)','2');
insert into instruction VALUES('0x0029','fsub       ',' {REG,REG,FLOAT}     ',' Float Subtract (op1=op2-op3)','2');
insert into instruction VALUES('0x002a','fsub       ',' {REG,FLOAT,REG}     ',' Float Subtract (op1=op2-op3)','2');
insert into instruction VALUES('0x00d5','ftob       ',' {REG}               ',' Set register boolean (int 1 or 0) value from its float value','2');
insert into instruction VALUES('0x00d4','ftoi       ',' {REG}               ',' Set register int value from its float value','2');
insert into instruction VALUES('0x00d2','ftos       ',' {REG}               ',' Set register string value from its float value','2');
insert into instruction VALUES('0x0074','feq        ',' {REG,REG,REG}       ',' Float Equals op1=(op2==op3)','2');
insert into instruction VALUES('0x0062','getstrpos  ',' {REG,REG}           ',' Get String (op2) charpos into op1','9');
insert into instruction VALUES('0x00a2','gmap       ',' {REG,REG}           ',' Map op1 to global var name in op2','9');
insert into instruction VALUES('0x00fd','gettp      ',' {REG,REG}           ',' gets the register type flag (op1 = op2.typeflag)','7');
insert into instruction VALUES('0x00a3','gmap       ',' {REG,STRING}        ',' Map op1 to global var name op2','7');
insert into instruction VALUES('0x00e0','getbyte    ',' {REG,REG,REG}       ',' get byte  (op1=op2(op3)','9');
insert into instruction VALUES('0x005f','hexchar    ',' {REG,REG,REG}       ',' op1 (as hex) = op2[op3]','9');
insert into instruction VALUES('0x000f','iadd       ',' {REG,REG,REG}       ',' Integer Add (op1=op2+op3)','1');
insert into instruction VALUES('0x0010','iadd       ',' {REG,REG,INT}       ',' Integer Add (op1=op2+op3)','1');
insert into instruction VALUES('0x0068','igt        ',' {REG,REG,REG}       ',' Int Greater than op1=(op2>op3)','1');
insert into instruction VALUES('0x0040','iand       ',' {REG,REG,REG}       ',' bit wise and of 2 integers (op1=op2&op3)','1');
insert into instruction VALUES('0x00ba','icopy      ',' {REG,REG}           ',' Copy Integer op2 to op1','1');
insert into instruction VALUES('0x001c','idiv       ',' {REG,REG,REG}       ',' Integer Divide (op1=op2/op3)','1');
insert into instruction VALUES('0x001d','idiv       ',' {REG,REG,INT}       ',' Integer Divide (op1=op2/op3)','1');
insert into instruction VALUES('0x001e','idiv       ',' {REG,INT,REG}       ',' Integer Divide (op1=op2/op3)','1');
insert into instruction VALUES('0x0064','ieq        ',' {REG,REG,REG}       ',' Int Equals op1=(op2==op3)','1');
insert into instruction VALUES('0x0065','ieq        ',' {REG,REG,INT}       ',' Int Equals op1=(op2==op3)','1');
insert into instruction VALUES('0x0069','igt        ',' {REG,REG,INT}       ',' Int Greater than op1=(op2>op3)','1');
insert into instruction VALUES('0x006b','igte       ',' {REG,REG,REG}       ',' Int Greater than equals op1=(op2>=op3)','1');
insert into instruction VALUES('0x0041','iand       ',' {REG,REG,INT}       ',' bit wise and of 2 integers (op1=op2&op3)','3');
insert into instruction VALUES('0x006c','igte       ',' {REG,REG,INT}       ',' Int Greater than equals op1=(op2>=op3)','1');
insert into instruction VALUES('0x006d','igte       ',' {REG,INT,REG}       ',' Int Greater than equals op1=(op2>=op3)','1');
insert into instruction VALUES('0x006e','ilt        ',' {REG,REG,REG}       ',' Int Less than op1=(op2<op3)','1');
insert into instruction VALUES('0x006f','ilt        ',' {REG,REG,INT}       ',' Int Less than op1=(op2<op3)','1');
insert into instruction VALUES('0x0070','ilt        ',' {REG,INT,REG}       ',' Int Less than op1=(op2<op3)','1');
insert into instruction VALUES('0x0071','ilte       ',' {REG,REG,REG}       ',' Int Less than equals op1=(op2<=op3)','1');
insert into instruction VALUES('0x0072','ilte       ',' {REG,REG,INT}       ',' Int Less than equals op1=(op2<=op3)','1');
insert into instruction VALUES('0x0073','ilte       ',' {REG,INT,REG}       ',' Int Less than equals op1=(op2<=op3)','1');
insert into instruction VALUES('0x0021','imod       ',' {REG,REG,REG}       ',' Integer Modulo (op1=op2%op3)','1');
insert into instruction VALUES('0x0022','imod       ',' {REG,REG,INT}       ',' Integer Modulo (op1=op2%op3)','1');
insert into instruction VALUES('0x0023','imod       ',' {REG,INT,REG}       ',' Integer Modulo (op1=op2&op3)','1');
insert into instruction VALUES('0x0018','imult      ',' {REG,REG,REG}       ',' Integer Multiply (op1=op2*op3)','1');
insert into instruction VALUES('0x0019','imult      ',' {REG,REG,INT}       ',' Integer Multiply (op1=op2*op3)','1');
insert into instruction VALUES('0x0038','inc        ',' {REG}               ',' Increment Int (op1++)','1');
insert into instruction VALUES('0x003a','inc0       ',' no operand          ',' Increment R0++ Int','1');
insert into instruction VALUES('0x003c','inc1       ',' no operand          ',' Increment R1++ Int','1');
insert into instruction VALUES('0x0066','ine        ',' {REG,REG,REG}       ',' Int Not equals op1=(op2!=op3)','1');
insert into instruction VALUES('0x006a','igt        ',' {REG,INT,REG}       ',' Int Greater than op1=(op2>op3)','1');
insert into instruction VALUES('0x0067','ine        ',' {REG,REG,INT}       ',' Int Not equals op1=(op2!=op3)','1');
insert into instruction VALUES('0x004a','inot       ',' {REG,REG}           ',' inverts all bits of an integer (op1=~op2)','3');
insert into instruction VALUES('0x004b','inot       ',' {REG,INT}           ',' inverts all bits of an integer (op1=~op2)','3');
insert into instruction VALUES('0x0042','ior        ',' {REG,REG,REG}       ',' bit wise or of 2 integers (op1=op2|op3)','3');
insert into instruction VALUES('0x0043','ior        ',' {REG,REG,INT}       ',' bit wise or of 2 integers (op1=op2|op3)','3');
insert into instruction VALUES('0x00e2','ipow       ',' {REG,REG,REG}       ',' op1=op2**op3','1');
insert into instruction VALUES('0x00e3','ipow       ',' {REG,REG,INT}       ',' op1=op2**op3','1');
insert into instruction VALUES('0x00e4','ipow       ',' {REG,INT,REG}       ',' op1=op2**op3','1');
insert into instruction VALUES('0x0107','irand      ',' {REG,REG}           ',' random number random, op1=irand(op2)','3');
insert into instruction VALUES('0x0108','irand      ',' {REG,INT}           ',' random number random, op1=irand(op2)','3');
insert into instruction VALUES('0x00fb','isex       ',' {REG}               ',' dec op1 = -op1 (sign change)','1');
insert into instruction VALUES('0x0046','ishl       ',' {REG,REG,REG}       ',' bit wise shift logical left of integer (op1=op2<<op3)','1');
insert into instruction VALUES('0x0047','ishl       ',' {REG,REG,INT}       ',' bit wise shift logical left of integer (op1=op2<<op3)','1');
insert into instruction VALUES('0x0048','ishr       ',' {REG,REG,REG}       ',' bit wise shift logical right of integer (op1=op2>>op3)','1');
insert into instruction VALUES('0x0049','ishr       ',' {REG,REG,INT}       ',' bit wise shift logical right of integer (op1=op2>>op3)','1');
insert into instruction VALUES('0x0013','isub       ',' {REG,REG,REG}       ',' Integer Subtract (op1=op2-op3)','1');
insert into instruction VALUES('0x0014','isub       ',' {REG,REG,INT}       ',' Integer Subtract (op1=op2-op3)','1');
insert into instruction VALUES('0x0015','isub       ',' {REG,INT,REG}       ',' Integer Subtract (op1=op2-op3)','1');
insert into instruction VALUES('0x00d3','itof       ',' {REG}               ',' Set register float value from its int value','1');
insert into instruction VALUES('0x00d1','itos       ',' {REG}               ',' Set register string value from its int value','9');
insert into instruction VALUES('0x0044','ixor       ',' {REG,REG,REG}       ',' bit wise exclusive OR of 2 integers (op1=op2^op3)','3');
insert into instruction VALUES('0x0045','ixor       ',' {REG,REG,INT}       ',' bit wise exclusive OR of 2 integers (op1=op2^op3)','3');
insert into instruction VALUES('0x003e','inc2       ',' no operand          ',' Increment R2++ Int','1');
insert into instruction VALUES('0x00bd','linkattr   ',' {REG,REG,REG}       ',' Link attribute op3 of op2 to op1','7');
insert into instruction VALUES('0x00c2','load       ',' {REG,INT}           ',' Load op1 with op2','1');
insert into instruction VALUES('0x00be','linkattr   ',' {REG,REG,INT}       ',' Link attribute op3 of op2 to op1','7');
insert into instruction VALUES('0x00ff','loadsettp  ',' {REG,INT,INT}       ',' load register and sets the register type flag load op1=op2 (o','1');
insert into instruction VALUES('0x00c3','load       ',' {REG,FLOAT}         ',' Load op1 with op2','2');
insert into instruction VALUES('0x00c5','load       ',' {REG,CHAR}          ',' Load op1 with op2','9');
insert into instruction VALUES('0x00c4','load       ',' {REG,STRING}        ',' Load op1 with op2','9');
insert into instruction VALUES('0x00bf','link       ',' {REG,REG}           ',' Link op2 to op1','4');
insert into instruction VALUES('0x0100','loadsettp  ',' {REG,FLOAT,INT}     ',' load register and sets the register type flag load op1=op2 (o','3');
insert into instruction VALUES('0x0101','loadsettp  ',' {REG,STRING,INT}    ',' load register and sets the register type flag load op1=op2 (o','3');
insert into instruction VALUES('0x0004','metaloadedm','modules {REG}        ','        Loaded Modules (op1 = array loaded modules)','7');
insert into instruction VALUES('0x009d','map        ',' {REG,STRING}        ',' Map op1 to var name op2','7');
insert into instruction VALUES('0x009c','map        ',' {REG,REG}           ',' Map op1 to var name in op2','7');
insert into instruction VALUES('0x0007','metadecodei','inst {REG,REG}       ','     Decode opcode (op1 decoded op2)','7');
insert into instruction VALUES('0x000e','metaloadcal','lleraddr {REG}       ','         Load caller address object to op1','7');
insert into instruction VALUES('0x000c','metaloaddat','ta {REG,REG,REG}     ','   Load Metadata (op1 = (metadata)op2[op3])','7');
insert into instruction VALUES('0x0005','metaloadedp','procs {REG,REG}      ','      Loaded Procedures (op1 = array procedures in module op2)','7');
insert into instruction VALUES('0x0009','metaloadfop','perand {REG,REG,REG} ','       Load Float Operand (op1 = (float)op2[op3])','7');
insert into instruction VALUES('0x0006','metaloadins','st {REG,REG,REG}     ','   Load Instruction Code (op1 = (inst)op2[op3])','7');
insert into instruction VALUES('0x0008','metaloadiop','perand {REG,REG,REG} ','       Load Integer/Index Operand (op1 = (int)op2[op3])','7');
insert into instruction VALUES('0x0003','metaloadmod','dule {REG,REG}       ','     Load Module (op1 = module num of last loaded module in rx','7');
insert into instruction VALUES('0x000b','metaloadpop','perand {REG,REG,REG} ','       Load Procedure Operand (op1 = (proc)op2[op3])','7');
insert into instruction VALUES('0x000a','metaloadsop','perand {REG,REG,REG} ','       Load String Operand (op1 = (string)op2[op3])','7');
insert into instruction VALUES('0x00b7','move       ',' {REG,REG}           ',' Move op2 to op1 (Deprecated use swap)','7');
insert into instruction VALUES('0x009a','mtime      ',' {REG}               ',' Put time in microseconds into op1','6');
insert into instruction VALUES('0x0030','multf      ',' {REG,REG,REG}       ',' Convert and Multiply to Float (op1=op2*op3) (Deprecated)','2');
insert into instruction VALUES('0x001a','multi      ',' {REG,REG,REG}       ',' Convert and Multiply to Integer (op1=op2*op3) (Deprecated)','2');
insert into instruction VALUES('0x000d','metalinkpre','eg {REG,REG}         ','   Link parent-frame-register[op2] to op1','7');
insert into instruction VALUES('0x001b','multi      ',' {REG,REG,INT}       ',' Convert and Multiply to Integer (op1=op2*op3) (Deprecated)','1');
insert into instruction VALUES('0x0031','multf      ',' {REG,REG,FLOAT}     ',' Convert and Multiply to Float (op1=op2*op3) (Deprecated)','2');
insert into instruction VALUES('0x00a4','nsmap      ',' {REG,REG,REG}       ',' Map op1 to namespace in op2 var name in op3','7');
insert into instruction VALUES('0x00a7','nsmap      ',' {REG,STRING,REG}    ',' Map op1 to namespace op2 var name in op3','7');
insert into instruction VALUES('0x00a5','nsmap      ',' {REG,REG,STRING}    ',' Map op1 to namespace in op2 var name op3','7');
insert into instruction VALUES('0x00c1','null       ',' {REG}               ',' Null op1','4');
insert into instruction VALUES('0x00a6','nsmap      ',' {REG,STRING,STRING} ',' Map op1 to namespace op2 var name op3','7');
insert into instruction VALUES('0x0098','not        ',' {REG,REG}           ',' Logical (int) not op1=!op2','3');
insert into instruction VALUES('0x0105','opendll    ',' {REG,REG,REG}       ',' open DLL','5');
insert into instruction VALUES('0x0097','or         ',' {REG,REG,REG}       ',' Logical (int) or op1=(op2 || op3)','3');
insert into instruction VALUES('0x00df','padstr     ',' {REG,REG,REG}       ',' set op1=op2[repeated op3 times]','9');
insert into instruction VALUES('0x00a0','pmap       ',' {REG,REG}           ',' Map op1 to parent var name in op2','7');
insert into instruction VALUES('0x00a1','pmap       ',' {REG,STRING}        ',' Map op1 to parent var name op2','7');
insert into instruction VALUES('0x0060','poschar    ',' {REG,REG,REG}       ',' op1 = position of op3 in op2','9');
insert into instruction VALUES('0x00b0','ret        ',' {FLOAT}             ',' Return op1','4');
insert into instruction VALUES('0x00ad','ret        ',' no operand          ',' Return VOID','4');
insert into instruction VALUES('0x00ae','ret        ',' {REG}               ',' Return op1','4');
insert into instruction VALUES('0x00af','ret        ',' {INT}               ',' Return op1','4');
insert into instruction VALUES('0x00b1','ret        ',' {CHAR}              ',' Return op1','4');
insert into instruction VALUES('0x00b2','ret        ',' {STRING}            ',' Return op1','4');
insert into instruction VALUES('0x0086','rseq       ',' {REG,REG,REG}       ',' non strict String Equals op1=(op2=op3)','9');
insert into instruction VALUES('0x0087','rseq       ',' {REG,REG,STRING}    ',' non strict String Equals op1=(op2=op3)','9');
insert into instruction VALUES('0x00cd','readline   ',' {REG}               ',' Read Line to op1','5');
insert into instruction VALUES('0x0054','sappend    ',' {REG,REG}           ',' String Append with space (op1=op1||op2)','9');
insert into instruction VALUES('0x00c6','say        ',' {REG}               ',' Say op1','5');
insert into instruction VALUES('0x00c9','say        ',' {INT}               ',' Say op1','5');
insert into instruction VALUES('0x00cc','say        ',' {CHAR}              ',' Say op1','5');
insert into instruction VALUES('0x00cb','say        ',' {STRING}            ',' Say op1','5');
insert into instruction VALUES('0x00c7','sayx       ',' {REG}               ',' Say op1 without line feed','5');
insert into instruction VALUES('0x00c8','sayx       ',' {STRING}            ',' Say op1 (as string) without line feed','5');
insert into instruction VALUES('0x004c','sconcat    ',' {REG,REG,REG}       ',' String Concat with space (op1=op2||op3)','9');
insert into instruction VALUES('0x004d','sconcat    ',' {REG,REG,STRING}    ',' String Concat with space (op1=op2||op3)','9');
insert into instruction VALUES('0x004e','sconcat    ',' {REG,STRING,REG}    ',' String Concat with space (op1=op2||op3)','9');
insert into instruction VALUES('0x00bc','scopy      ',' {REG,REG}           ',' Copy String op2 to op1','9');
insert into instruction VALUES('0x0084','seq        ',' {REG,REG,REG}       ',' String Equals op1=(op2==op3)','9');
insert into instruction VALUES('0x0085','seq        ',' {REG,REG,STRING}    ',' String Equals op1=(op2==op3)','9');
insert into instruction VALUES('0x0102','setortp    ',' {REG,INT}           ',' or the register type flag (op1.typeflag = op1.typeflag || op2','3');
insert into instruction VALUES('0x0061','setstrpos  ',' {REG,REG}           ',' Set String (op1) charpos set to op2','9');
insert into instruction VALUES('0x00fe','settp      ',' {REG,INT}           ',' sets the register type flag (op1.typeflag = op2)','7');
insert into instruction VALUES('0x008a','sgt        ',' {REG,REG,REG}       ',' String Greater than op1=(op2>op3)','9');
insert into instruction VALUES('0x008b','sgt        ',' {REG,REG,STRING}    ',' String Greater than op1=(op2>op3)','9');
insert into instruction VALUES('0x008c','sgt        ',' {REG,STRING,REG}    ',' String Greater than op1=(op2>op3)','9');
insert into instruction VALUES('0x008d','sgte       ',' {REG,REG,REG}       ',' String Greater than equals op1=(op2>=op3)','9');
insert into instruction VALUES('0x008e','sgte       ',' {REG,REG,STRING}    ',' String Greater than equals op1=(op2>=op3)','9');
insert into instruction VALUES('0x008f','sgte       ',' {REG,STRING,REG}    ',' String Greater than equals op1=(op2>=op3)','9');
insert into instruction VALUES('0x0090','slt        ',' {REG,REG,REG}       ',' String Less than op1=(op2<op3)','9');
insert into instruction VALUES('0x0091','slt        ',' {REG,REG,STRING}    ',' String Less than op1=(op2<op3)','9');
insert into instruction VALUES('0x0092','slt        ',' {REG,STRING,REG}    ',' String Less than op1=(op2<op3)','9');
insert into instruction VALUES('0x0093','slte       ',' {REG,REG,REG}       ',' String Less than equals op1=(op2<=op3)','9');
insert into instruction VALUES('0x0094','slte       ',' {REG,REG,STRING}    ',' String Less than equals op1=(op2<=op3)','9');
insert into instruction VALUES('0x0095','slte       ',' {REG,STRING,REG}    ',' String Less than equals op1=(op2<=op3)','9');
insert into instruction VALUES('0x0088','sne        ',' {REG,REG,REG}       ',' String Not equals op1=(op2!=op3)','9');
insert into instruction VALUES('0x0089','sne        ',' {REG,REG,STRING}    ',' String Not equals op1=(op2!=op3)','9');
insert into instruction VALUES('0x00d6','stof       ',' {REG}               ',' Set register float value from its string value','9');
insert into instruction VALUES('0x00d7','stoi       ',' {REG}               ',' Set register int value from its string value','9');
insert into instruction VALUES('0x005e','strchar    ',' {REG,REG}           ',' op1 (as int) = op2[charpos]','9');
insert into instruction VALUES('0x005d','strchar    ',' {REG,REG,REG}       ',' op1 (as int) = op2[op3]','9');
insert into instruction VALUES('0x005c','strlen     ',' {REG,REG}           ',' String Length op1 = length(op2)','9');
insert into instruction VALUES('0x00d9','strlower   ',' {REG,REG}           ',' Set string to lower case value','9');
insert into instruction VALUES('0x00da','strupper   ',' {REG,REG}           ',' Set string to upper case value','9');
insert into instruction VALUES('0x002b','subf       ',' {REG,REG,REG}       ',' Convert and Subtract to Float (op1=op2-op3) (Deprecated)','2');
insert into instruction VALUES('0x002c','subf       ',' {REG,REG,FLOAT}     ',' Convert and Subtract to Float (op1=op2-op3) (Deprecated)','2');
insert into instruction VALUES('0x002d','subf       ',' {REG,FLOAT,REG}     ',' Convert and Subtract to Float (op1=op2-op3) (Deprecated)','2');
insert into instruction VALUES('0x0016','subi       ',' {REG,REG,REG}       ',' Convert and Subtract to Integer (op1=op2-op3) (Deprecated)','2');
insert into instruction VALUES('0x0017','subi       ',' {REG,REG,INT}       ',' Convert and Subtract to Integer (op1=op2-op3) (Deprecated)','2');
insert into instruction VALUES('0x00de','substcut   ',' {REG,REG}           ',' set op1=substr(op1,,op2) cuts off op1 after position op3','9');
insert into instruction VALUES('0x0063','substr     ',' {REG,REG,REG}       ',' op1 = op2[charpos]...op2[charpos+op3-1]','9');
insert into instruction VALUES('0x00dd','substring  ',' {REG,REG,REG}       ',' set op1=substr(op2,op3) remaining string','9');
insert into instruction VALUES('0x00b8','swap       ',' {REG,REG}           ',' Swap op1 and op2','3');
insert into instruction VALUES('0x00ca','say        ',' {FLOAT}             ',' Say op1','5');
insert into instruction VALUES('0x0099','time       ',' {REG}               ',' Put time into op1','6');
insert into instruction VALUES('0x00db','transchar  ',' {REG,REG,REG}       ',' replace op1 if it is in op3-list by char in op2-list','9');
insert into instruction VALUES('0x0056','triml      ',' {REG,REG}           ',' Trim String (op1) from Left by (op2) Chars','9');
insert into instruction VALUES('0x0059','triml      ',' {REG,REG,REG}       ',' Trim String (op2) from Left by (op3) Chars into op1','9');
insert into instruction VALUES('0x0057','trimr      ',' {REG,REG}           ',' Trim String (op1) from Right by (op2) Chars','9');
insert into instruction VALUES('0x005a','trimr      ',' {REG,REG,REG}       ',' Trim String (op2) from Right by (op3) Chars into op1','9');
insert into instruction VALUES('0x0058','trunc      ',' {REG,REG}           ',' Trunc String (op1) to (op2) Chars','9');
insert into instruction VALUES('0x005b','trunc      ',' {REG,REG,REG}       ',' Trunc String (op2) to (op3) Chars into op1','9');
insert into instruction VALUES('0x00c0','unlink     ',' {REG}               ',' Unlink op1','7');
insert into instruction VALUES('0x00a8','unmap      ',' {REG}               ',' Unmap op1','7');
insert into instruction VALUES('0x009b','xtime      ',' {REG,STRING}        ',' put special time properties into op1','6');
