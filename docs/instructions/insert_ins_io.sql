insert into instruction values('0x012d','freadb','{REG,REG,REG}',' op1(binary) = fread op2 file*(int) op3 bytes(int)');
insert into instruction values('0x012f','freadbyte','{REG,REG}',' op1 (int) = read byte op2 file*(int)');
insert into instruction values('0x0130','freadcdpt','{REG,REG}',' op1 (string and int) = read codepoint op2 file*(int)');
insert into instruction values('0x012e','freadline','{REG,REG}',' op1 (string) = read until newline op2 file*(int)');
insert into instruction values('0x0131','fwrite','{REG,REG}',' fwrite to op1 file*(int) from op2(string)');
insert into instruction values('0x0132','fwriteb','{REG,REG}',' fwrite to op1 file*(int) from op2(binary)');
insert into instruction values('0x0133','fwritebyte','{REG,REG}',' write byte to op1 file*(int) op2 source(int)');
insert into instruction values('0x0134','fwritecdpt','{REG,REG}',' write codepoint to op1 file*(int) op2 source(int)');
