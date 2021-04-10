



// ----- iadd Instruction Integer Add (op1=op2+op3) -----
instruction = src_inst("iadd", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&IADD_REG_REG_REG;
    else print_debug("Instruction IADD_REG_REG_REG not found\n");
  
// ----- iadd Instruction Integer Add (op1=op2+op3) -----
instruction = src_inst("iadd", OP_REG,OP_REG,OP_INT);
if (instruction) address_map[instruction->opcode] = &&IADD_REG_REG_INT;
    else print_debug("Instruction IADD_REG_REG_INT not found\n");
  
// ----- addi Instruction Convert and Add to Integer (op1=op2+op3) -----
instruction = src_inst("addi", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&ADDI_REG_REG_REG;
    else print_debug("Instruction ADDI_REG_REG_REG not found\n");
  
// ----- addi Instruction Convert and Add to Integer (op1=op2+op3) -----
instruction = src_inst("addi", OP_REG,OP_REG,OP_INT);
if (instruction) address_map[instruction->opcode] = &&ADDI_REG_REG_INT;
    else print_debug("Instruction ADDI_REG_REG_INT not found\n");
  
// ----- isub Instruction Integer Subtract (op1=op2-op3) -----
instruction = src_inst("isub", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&ISUB_REG_REG_REG;
    else print_debug("Instruction ISUB_REG_REG_REG not found\n");
  
// ----- isub Instruction Integer Subtract (op1=op2-op3) -----
instruction = src_inst("isub", OP_REG,OP_REG,OP_INT);
if (instruction) address_map[instruction->opcode] = &&ISUB_REG_REG_INT;
    else print_debug("Instruction ISUB_REG_REG_INT not found\n");
  
// ----- subi Instruction Convert and Subtract to Integer (op1=op2-op3) -----
instruction = src_inst("subi", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&SUBI_REG_REG_REG;
    else print_debug("Instruction SUBI_REG_REG_REG not found\n");
  
// ----- subi Instruction Convert and Subtract to Integer (op1=op2-op3) -----
instruction = src_inst("subi", OP_REG,OP_REG,OP_INT);
if (instruction) address_map[instruction->opcode] = &&SUBI_REG_REG_INT;
    else print_debug("Instruction SUBI_REG_REG_INT not found\n");
  
// ----- imult Instruction Integer Multiply (op1=op2*op3) -----
instruction = src_inst("imult", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&IMULT_REG_REG_REG;
    else print_debug("Instruction IMULT_REG_REG_REG not found\n");
  
// ----- imult Instruction Integer Multiply (op1=op2*op3) -----
instruction = src_inst("imult", OP_REG,OP_REG,OP_INT);
if (instruction) address_map[instruction->opcode] = &&IMULT_REG_REG_INT;
    else print_debug("Instruction IMULT_REG_REG_INT not found\n");
  
// ----- multi Instruction Convert and Multiply to Integer (op1=op2*op3) -----
instruction = src_inst("multi", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&MULTI_REG_REG_REG;
    else print_debug("Instruction MULTI_REG_REG_REG not found\n");
  
// ----- multi Instruction Convert and Multiply to Integer (op1=op2*op3) -----
instruction = src_inst("multi", OP_REG,OP_REG,OP_INT);
if (instruction) address_map[instruction->opcode] = &&MULTI_REG_REG_INT;
    else print_debug("Instruction MULTI_REG_REG_INT not found\n");
  
// ----- idiv Instruction Integer Divide (op1=op2/op3) -----
instruction = src_inst("idiv", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&IDIV_REG_REG_REG;
    else print_debug("Instruction IDIV_REG_REG_REG not found\n");
  
// ----- idiv Instruction Integer Divide (op1=op2/op3) -----
instruction = src_inst("idiv", OP_REG,OP_REG,OP_INT);
if (instruction) address_map[instruction->opcode] = &&IDIV_REG_REG_INT;
    else print_debug("Instruction IDIV_REG_REG_INT not found\n");
  
// ----- divi Instruction Convert and Divide to Integer (op1=op2/op3) -----
instruction = src_inst("divi", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&DIVI_REG_REG_REG;
    else print_debug("Instruction DIVI_REG_REG_REG not found\n");
  
// ----- divi Instruction Convert and Divide to Integer (op1=op2/op3) -----
instruction = src_inst("divi", OP_REG,OP_REG,OP_INT);
if (instruction) address_map[instruction->opcode] = &&DIVI_REG_REG_INT;
    else print_debug("Instruction DIVI_REG_REG_INT not found\n");
  
// ----- fadd Instruction Float Add (op1=op2+op3) -----
instruction = src_inst("fadd", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&FADD_REG_REG_REG;
    else print_debug("Instruction FADD_REG_REG_REG not found\n");
  
// ----- fadd Instruction Float Add (op1=op2+op3) -----
instruction = src_inst("fadd", OP_REG,OP_REG,OP_FLOAT);
if (instruction) address_map[instruction->opcode] = &&FADD_REG_REG_FLOAT;
    else print_debug("Instruction FADD_REG_REG_FLOAT not found\n");
  
// ----- addf Instruction Convert and Add to Float (op1=op2+op3) -----
instruction = src_inst("addf", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&ADDF_REG_REG_REG;
    else print_debug("Instruction ADDF_REG_REG_REG not found\n");
  
// ----- addf Instruction Convert and Add to Float (op1=op2+op3) -----
instruction = src_inst("addf", OP_REG,OP_REG,OP_FLOAT);
if (instruction) address_map[instruction->opcode] = &&ADDF_REG_REG_FLOAT;
    else print_debug("Instruction ADDF_REG_REG_FLOAT not found\n");
  
// ----- fsub Instruction Float Subtract (op1=op2-op3) -----
instruction = src_inst("fsub", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&FSUB_REG_REG_REG;
    else print_debug("Instruction FSUB_REG_REG_REG not found\n");
  
// ----- fsub Instruction Float Subtract (op1=op2-op3) -----
instruction = src_inst("fsub", OP_REG,OP_REG,OP_FLOAT);
if (instruction) address_map[instruction->opcode] = &&FSUB_REG_REG_FLOAT;
    else print_debug("Instruction FSUB_REG_REG_FLOAT not found\n");
  
// ----- fsub Instruction Float Subtract (op1=op2-op3) -----
instruction = src_inst("fsub", OP_REG,OP_FLOAT,OP_REG);
if (instruction) address_map[instruction->opcode] = &&FSUB_REG_FLOAT_REG;
    else print_debug("Instruction FSUB_REG_FLOAT_REG not found\n");
  
// ----- subf Instruction Convert and Subtract to Float (op1=op2-op3) -----
instruction = src_inst("subf", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&SUBF_REG_REG_REG;
    else print_debug("Instruction SUBF_REG_REG_REG not found\n");
  
// ----- subf Instruction Convert and Subtract to Float (op1=op2-op3) -----
instruction = src_inst("subf", OP_REG,OP_REG,OP_FLOAT);
if (instruction) address_map[instruction->opcode] = &&SUBF_REG_REG_FLOAT;
    else print_debug("Instruction SUBF_REG_REG_FLOAT not found\n");
  
// ----- subf Instruction Convert and Subtract to Float (op1=op2-op3) -----
instruction = src_inst("subf", OP_REG,OP_FLOAT,OP_REG);
if (instruction) address_map[instruction->opcode] = &&SUBF_REG_FLOAT_REG;
    else print_debug("Instruction SUBF_REG_FLOAT_REG not found\n");
  
// ----- fmult Instruction Float Multiply (op1=op2*op3) -----
instruction = src_inst("fmult", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&FMULT_REG_REG_REG;
    else print_debug("Instruction FMULT_REG_REG_REG not found\n");
  
// ----- fmult Instruction Float Multiply (op1=op2*op3) -----
instruction = src_inst("fmult", OP_REG,OP_REG,OP_FLOAT);
if (instruction) address_map[instruction->opcode] = &&FMULT_REG_REG_FLOAT;
    else print_debug("Instruction FMULT_REG_REG_FLOAT not found\n");
  
// ----- multf Instruction Convert and Multiply to Float (op1=op2*op3) -----
instruction = src_inst("multf", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&MULTF_REG_REG_REG;
    else print_debug("Instruction MULTF_REG_REG_REG not found\n");
  
// ----- multf Instruction Convert and Multiply to Float (op1=op2*op3) -----
instruction = src_inst("multf", OP_REG,OP_REG,OP_FLOAT);
if (instruction) address_map[instruction->opcode] = &&MULTF_REG_REG_FLOAT;
    else print_debug("Instruction MULTF_REG_REG_FLOAT not found\n");
  
// ----- fdiv Instruction Float Divide (op1=op2/op3) -----
instruction = src_inst("fdiv", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&FDIV_REG_REG_REG;
    else print_debug("Instruction FDIV_REG_REG_REG not found\n");
  
// ----- fdiv Instruction Float Divide (op1=op2/op3) -----
instruction = src_inst("fdiv", OP_REG,OP_REG,OP_FLOAT);
if (instruction) address_map[instruction->opcode] = &&FDIV_REG_REG_FLOAT;
    else print_debug("Instruction FDIV_REG_REG_FLOAT not found\n");
  
// ----- fdiv Instruction Float Divide (op1=op2/op3) -----
instruction = src_inst("fdiv", OP_REG,OP_FLOAT,OP_REG);
if (instruction) address_map[instruction->opcode] = &&FDIV_REG_FLOAT_REG;
    else print_debug("Instruction FDIV_REG_FLOAT_REG not found\n");
  
// ----- divf Instruction Convert and Divide to Float (op1=op2/op3) -----
instruction = src_inst("divf", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&DIVF_REG_REG_REG;
    else print_debug("Instruction DIVF_REG_REG_REG not found\n");
  
// ----- divf Instruction Convert and Divide to Float (op1=op2/op3) -----
instruction = src_inst("divf", OP_REG,OP_REG,OP_FLOAT);
if (instruction) address_map[instruction->opcode] = &&DIVF_REG_REG_FLOAT;
    else print_debug("Instruction DIVF_REG_REG_FLOAT not found\n");
  
// ----- divf Instruction Convert and Divide to Float (op1=op2/op3) -----
instruction = src_inst("divf", OP_REG,OP_FLOAT,OP_REG);
if (instruction) address_map[instruction->opcode] = &&DIVF_REG_FLOAT_REG;
    else print_debug("Instruction DIVF_REG_FLOAT_REG not found\n");
  
// ----- sconcat Instruction String Concat (op1=op2||op3) -----
instruction = src_inst("sconcat", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&SCONCAT_REG_REG_REG;
    else print_debug("Instruction SCONCAT_REG_REG_REG not found\n");
  
// ----- sconcat Instruction String Concat (op1=op2||op3) -----
instruction = src_inst("sconcat", OP_REG,OP_REG,OP_STRING);
if (instruction) address_map[instruction->opcode] = &&SCONCAT_REG_REG_STRING;
    else print_debug("Instruction SCONCAT_REG_REG_STRING not found\n");
  
// ----- sconcat Instruction String Concat (op1=op2||op3) -----
instruction = src_inst("sconcat", OP_REG,OP_STRING,OP_REG);
if (instruction) address_map[instruction->opcode] = &&SCONCAT_REG_STRING_REG;
    else print_debug("Instruction SCONCAT_REG_STRING_REG not found\n");
  
// ----- concats Instruction Convert and Concat to String (op1=op2||op3) -----
instruction = src_inst("concats", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&CONCATS_REG_REG_REG;
    else print_debug("Instruction CONCATS_REG_REG_REG not found\n");
  
// ----- concats Instruction Convert and Concat to String (op1=op2||op3) -----
instruction = src_inst("concats", OP_REG,OP_REG,OP_STRING);
if (instruction) address_map[instruction->opcode] = &&CONCATS_REG_REG_STRING;
    else print_debug("Instruction CONCATS_REG_REG_STRING not found\n");
  
// ----- concats Instruction Convert and Concat to String (op1=op2||op3) -----
instruction = src_inst("concats", OP_REG,OP_STRING,OP_REG);
if (instruction) address_map[instruction->opcode] = &&CONCATS_REG_STRING_REG;
    else print_debug("Instruction CONCATS_REG_STRING_REG not found\n");
  
// ----- inc Instruction Increment Int (op1++) -----
instruction = src_inst("inc", OP_REG,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&INC_REG;
    else print_debug("Instruction INC_REG not found\n");
  
// ----- dec Instruction Decrement Int (op1--) -----
instruction = src_inst("dec", OP_REG,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&DEC_REG;
    else print_debug("Instruction DEC_REG not found\n");
  
// ----- inc0 Instruction Increment R0++ Int -----
instruction = src_inst("inc0", OP_NONE,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&INC0;
    else print_debug("Instruction INC0 not found\n");
  
// ----- dec0 Instruction Decrement R0-- Int -----
instruction = src_inst("dec0", OP_NONE,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&DEC0;
    else print_debug("Instruction DEC0 not found\n");
  
// ----- inc1 Instruction Increment R0++ Int -----
instruction = src_inst("inc1", OP_NONE,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&INC1;
    else print_debug("Instruction INC1 not found\n");
  
// ----- dec1 Instruction Decrement R0-- Int -----
instruction = src_inst("dec1", OP_NONE,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&DEC1;
    else print_debug("Instruction DEC1 not found\n");
  
// ----- inc2 Instruction Increment R0++ Int -----
instruction = src_inst("inc2", OP_NONE,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&INC2;
    else print_debug("Instruction INC2 not found\n");
  
// ----- dec2 Instruction Decrement R0-- Int -----
instruction = src_inst("dec2", OP_NONE,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&DEC2;
    else print_debug("Instruction DEC2 not found\n");
  
// ----- triml Instruction Trim String (op1) from Left by (op2) Chars -----
instruction = src_inst("triml", OP_REG,OP_REG,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&TRIML_REG_REG;
    else print_debug("Instruction TRIML_REG_REG not found\n");
  
// ----- trimr Instruction Trim String (op1) from Right by (op2) Chars -----
instruction = src_inst("trimr", OP_REG,OP_REG,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&TRIMR_REG_REG;
    else print_debug("Instruction TRIMR_REG_REG not found\n");
  
// ----- strlen Instruction String Length op1 = length(op2) -----
instruction = src_inst("strlen", OP_REG,OP_REG,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&STRLEN_REG_REG;
    else print_debug("Instruction STRLEN_REG_REG not found\n");
  
// ----- str2char Instruction String to Char op1 = op2[op3] -----
instruction = src_inst("str2char", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&STR2CHAR_REG_REG_REG;
    else print_debug("Instruction STR2CHAR_REG_REG_REG not found\n");
  
// ----- str2int Instruction String to Int op1 = op2[op3] -----
instruction = src_inst("str2int", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&STR2INT_REG_REG_REG;
    else print_debug("Instruction STR2INT_REG_REG_REG not found\n");
  
// ----- ieq Instruction Int Equals op1=(op2==op3) -----
instruction = src_inst("ieq", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&IEQ_REG_REG_REG;
    else print_debug("Instruction IEQ_REG_REG_REG not found\n");
  
// ----- ieq Instruction Int Equals op1=(op2==op3) -----
instruction = src_inst("ieq", OP_REG,OP_REG,OP_INT);
if (instruction) address_map[instruction->opcode] = &&IEQ_REG_REG_INT;
    else print_debug("Instruction IEQ_REG_REG_INT not found\n");
  
// ----- ine Instruction Int Not equals op1=(op2!=op3) -----
instruction = src_inst("ine", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&INE_REG_REG_REG;
    else print_debug("Instruction INE_REG_REG_REG not found\n");
  
// ----- ine Instruction Int Not equals op1=(op2!=op3) -----
instruction = src_inst("ine", OP_REG,OP_REG,OP_INT);
if (instruction) address_map[instruction->opcode] = &&INE_REG_REG_INT;
    else print_debug("Instruction INE_REG_REG_INT not found\n");
  
// ----- igt Instruction Int Greater than op1=(op2>op3) -----
instruction = src_inst("igt", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&IGT_REG_REG_REG;
    else print_debug("Instruction IGT_REG_REG_REG not found\n");
  
// ----- igt Instruction Int Greater than op1=(op2>op3) -----
instruction = src_inst("igt", OP_REG,OP_REG,OP_INT);
if (instruction) address_map[instruction->opcode] = &&IGT_REG_REG_INT;
    else print_debug("Instruction IGT_REG_REG_INT not found\n");
  
// ----- igt Instruction Int Greater than op1=(op2>op3) -----
instruction = src_inst("igt", OP_REG,OP_INT,OP_REG);
if (instruction) address_map[instruction->opcode] = &&IGT_REG_INT_REG;
    else print_debug("Instruction IGT_REG_INT_REG not found\n");
  
// ----- igte Instruction Int Greater than equals op1=(op2>=op3) -----
instruction = src_inst("igte", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&IGTE_REG_REG_REG;
    else print_debug("Instruction IGTE_REG_REG_REG not found\n");
  
// ----- igte Instruction Int Greater than equals op1=(op2>=op3) -----
instruction = src_inst("igte", OP_REG,OP_REG,OP_INT);
if (instruction) address_map[instruction->opcode] = &&IGTE_REG_REG_INT;
    else print_debug("Instruction IGTE_REG_REG_INT not found\n");
  
// ----- igte Instruction Int Greater than equals op1=(op2>=op3) -----
instruction = src_inst("igte", OP_REG,OP_INT,OP_REG);
if (instruction) address_map[instruction->opcode] = &&IGTE_REG_INT_REG;
    else print_debug("Instruction IGTE_REG_INT_REG not found\n");
  
// ----- ilt Instruction Int Less than op1=(op2<op3) -----
instruction = src_inst("ilt", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&ILT_REG_REG_REG;
    else print_debug("Instruction ILT_REG_REG_REG not found\n");
  
// ----- ilt Instruction Int Less than op1=(op2<op3) -----
instruction = src_inst("ilt", OP_REG,OP_REG,OP_INT);
if (instruction) address_map[instruction->opcode] = &&ILT_REG_REG_INT;
    else print_debug("Instruction ILT_REG_REG_INT not found\n");
  
// ----- ilt Instruction Int Less than op1=(op2<op3) -----
instruction = src_inst("ilt", OP_REG,OP_INT,OP_REG);
if (instruction) address_map[instruction->opcode] = &&ILT_REG_INT_REG;
    else print_debug("Instruction ILT_REG_INT_REG not found\n");
  
// ----- ilte Instruction Int Less than equals op1=(op2<=op3) -----
instruction = src_inst("ilte", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&ILTE_REG_REG_REG;
    else print_debug("Instruction ILTE_REG_REG_REG not found\n");
  
// ----- ilte Instruction Int Less than equals op1=(op2<=op3) -----
instruction = src_inst("ilte", OP_REG,OP_REG,OP_INT);
if (instruction) address_map[instruction->opcode] = &&ILTE_REG_REG_INT;
    else print_debug("Instruction ILTE_REG_REG_INT not found\n");
  
// ----- ilte Instruction Int Less than equals op1=(op2<=op3) -----
instruction = src_inst("ilte", OP_REG,OP_INT,OP_REG);
if (instruction) address_map[instruction->opcode] = &&ILTE_REG_INT_REG;
    else print_debug("Instruction ILTE_REG_INT_REG not found\n");
  
// ----- iprime Instruction Int Prime op1 -----
instruction = src_inst("iprime", OP_REG,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&IPRIME_REG;
    else print_debug("Instruction IPRIME_REG not found\n");
  
// ----- fprime Instruction Float Prime op1 -----
instruction = src_inst("fprime", OP_REG,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&FPRIME_REG;
    else print_debug("Instruction FPRIME_REG not found\n");
  
// ----- sprime Instruction String Prime op1 -----
instruction = src_inst("sprime", OP_REG,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&SPRIME_REG;
    else print_debug("Instruction SPRIME_REG not found\n");
  
// ----- cprime Instruction Char Prime op1 -----
instruction = src_inst("cprime", OP_REG,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&CPRIME_REG;
    else print_debug("Instruction CPRIME_REG not found\n");
  
// ----- imaster Instruction Make int the master value for op1 -----
instruction = src_inst("imaster", OP_REG,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&IMASTER_REG;
    else print_debug("Instruction IMASTER_REG not found\n");
  
// ----- time Instruction Put time into op1 -----
instruction = src_inst("time", OP_REG,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&TIME_REG;
    else print_debug("Instruction TIME_REG not found\n");
  
// ----- map Instruction Map op1 to var name in op2 -----
instruction = src_inst("map", OP_REG,OP_REG,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&MAP_REG_REG;
    else print_debug("Instruction MAP_REG_REG not found\n");
  
// ----- map Instruction Map op1 to var name op2 -----
instruction = src_inst("map", OP_REG,OP_STRING,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&MAP_REG_STRING;
    else print_debug("Instruction MAP_REG_STRING not found\n");
  
// ----- pmap Instruction Map op1 to parent var name in op2 -----
instruction = src_inst("pmap", OP_REG,OP_REG,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&PMAP_REG_REG;
    else print_debug("Instruction PMAP_REG_REG not found\n");
  
// ----- pmap Instruction Map op1 to parent var name op2 -----
instruction = src_inst("pmap", OP_REG,OP_STRING,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&PMAP_REG_STRING;
    else print_debug("Instruction PMAP_REG_STRING not found\n");
  
// ----- gmap Instruction Map op1 to global var name in op2 -----
instruction = src_inst("gmap", OP_REG,OP_REG,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&GMAP_REG_REG;
    else print_debug("Instruction GMAP_REG_REG not found\n");
  
// ----- gmap Instruction Map op1 to global var name op2 -----
instruction = src_inst("gmap", OP_REG,OP_STRING,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&GMAP_REG_STRING;
    else print_debug("Instruction GMAP_REG_STRING not found\n");
  
// ----- nsmap Instruction Map op1 to namespace in op2 var name in op3 -----
instruction = src_inst("nsmap", OP_REG,OP_REG,OP_REG);
if (instruction) address_map[instruction->opcode] = &&NSMAP_REG_REG_REG;
    else print_debug("Instruction NSMAP_REG_REG_REG not found\n");
  
// ----- nsmap Instruction Map op1 to namespace in op2 var name op3 -----
instruction = src_inst("nsmap", OP_REG,OP_REG,OP_STRING);
if (instruction) address_map[instruction->opcode] = &&NSMAP_REG_REG_STRING;
    else print_debug("Instruction NSMAP_REG_REG_STRING not found\n");
  
// ----- nsmap Instruction Map op1 to namespace op2 var name op3 -----
instruction = src_inst("nsmap", OP_REG,OP_STRING,OP_STRING);
if (instruction) address_map[instruction->opcode] = &&NSMAP_REG_STRING_STRING;
    else print_debug("Instruction NSMAP_REG_STRING_STRING not found\n");
  
// ----- nsmap Instruction Map op1 to namespace op2 var name in op3 -----
instruction = src_inst("nsmap", OP_REG,OP_STRING,OP_REG);
if (instruction) address_map[instruction->opcode] = &&NSMAP_REG_STRING_REG;
    else print_debug("Instruction NSMAP_REG_STRING_REG not found\n");
  
// ----- unmap Instruction Unmap op1 -----
instruction = src_inst("unmap", OP_REG,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&UNMAP_REG;
    else print_debug("Instruction UNMAP_REG not found\n");
  
// ----- call Instruction Call procedure (op1()) -----
instruction = src_inst("call", OP_FUNC,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&CALL_FUNC;
    else print_debug("Instruction CALL_FUNC not found\n");
  
// ----- call Instruction Call procedure (op1=op2()) -----
instruction = src_inst("call", OP_REG,OP_FUNC,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&CALL_REG_FUNC;
    else print_debug("Instruction CALL_REG_FUNC not found\n");
  
// ----- call Instruction Call procedure (op1=op2(op3...) ) -----
instruction = src_inst("call", OP_REG,OP_FUNC,OP_REG);
if (instruction) address_map[instruction->opcode] = &&CALL_REG_FUNC_REG;
    else print_debug("Instruction CALL_REG_FUNC_REG not found\n");
  
// ----- ret Instruction Return NULL -----
instruction = src_inst("ret", OP_NONE,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&RET;
    else print_debug("Instruction RET not found\n");
  
// ----- ret Instruction Return op1 -----
instruction = src_inst("ret", OP_REG,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&RET_REG;
    else print_debug("Instruction RET_REG not found\n");
  
// ----- ret Instruction Return op1 -----
instruction = src_inst("ret", OP_INT,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&RET_INT;
    else print_debug("Instruction RET_INT not found\n");
  
// ----- ret Instruction Return op1 -----
instruction = src_inst("ret", OP_FLOAT,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&RET_FLOAT;
    else print_debug("Instruction RET_FLOAT not found\n");
  
// ----- ret Instruction Return op1 -----
instruction = src_inst("ret", OP_CHAR,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&RET_CHAR;
    else print_debug("Instruction RET_CHAR not found\n");
  
// ----- ret Instruction Return op1 -----
instruction = src_inst("ret", OP_STRING,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&RET_STRING;
    else print_debug("Instruction RET_STRING not found\n");
  
// ----- br Instruction Branch to op1 -----
instruction = src_inst("br", OP_ID,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&BR_ID;
    else print_debug("Instruction BR_ID not found\n");
  
// ----- brt Instruction Branch to op1 if op2 true -----
instruction = src_inst("brt", OP_ID,OP_REG,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&BRT_ID_REG;
    else print_debug("Instruction BRT_ID_REG not found\n");
  
// ----- brf Instruction Branch to op1 if op2 false -----
instruction = src_inst("brf", OP_ID,OP_REG,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&BRF_ID_REG;
    else print_debug("Instruction BRF_ID_REG not found\n");
  
// ----- move Instruction Move op2 to op1 -----
instruction = src_inst("move", OP_REG,OP_REG,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&MOVE_REG_REG;
    else print_debug("Instruction MOVE_REG_REG not found\n");
  
// ----- copy Instruction Copy op2 to op1 -----
instruction = src_inst("copy", OP_REG,OP_REG,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&COPY_REG_REG;
    else print_debug("Instruction COPY_REG_REG not found\n");
  
// ----- link Instruction Link op2 to op1 -----
instruction = src_inst("link", OP_REG,OP_REG,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&LINK_REG_REG;
    else print_debug("Instruction LINK_REG_REG not found\n");
  
// ----- unlink Instruction Unlink op1 -----
instruction = src_inst("unlink", OP_REG,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&UNLINK_REG;
    else print_debug("Instruction UNLINK_REG not found\n");
  
// ----- null Instruction Null op1 -----
instruction = src_inst("null", OP_REG,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&NULL_REG;
    else print_debug("Instruction NULL_REG not found\n");
  
// ----- load Instruction Load op1 with op2 -----
instruction = src_inst("load", OP_REG,OP_INT,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&LOAD_REG_INT;
    else print_debug("Instruction LOAD_REG_INT not found\n");
  
// ----- load Instruction Load op1 with op2 -----
instruction = src_inst("load", OP_REG,OP_FLOAT,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&LOAD_REG_FLOAT;
    else print_debug("Instruction LOAD_REG_FLOAT not found\n");
  
// ----- load Instruction Load op1 with op2 -----
instruction = src_inst("load", OP_REG,OP_STRING,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&LOAD_REG_STRING;
    else print_debug("Instruction LOAD_REG_STRING not found\n");
  
// ----- load Instruction Load op1 with op2 -----
instruction = src_inst("load", OP_REG,OP_CHAR,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&LOAD_REG_CHAR;
    else print_debug("Instruction LOAD_REG_CHAR not found\n");
  
// ----- say Instruction Say op1 -----
instruction = src_inst("say", OP_REG,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&SAY_REG;
    else print_debug("Instruction SAY_REG not found\n");
  
// ----- say Instruction Say op1 -----
instruction = src_inst("say", OP_INT,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&SAY_INT;
    else print_debug("Instruction SAY_INT not found\n");
  
// ----- say Instruction Say op1 -----
instruction = src_inst("say", OP_FLOAT,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&SAY_FLOAT;
    else print_debug("Instruction SAY_FLOAT not found\n");
  
// ----- say Instruction Say op1 -----
instruction = src_inst("say", OP_STRING,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&SAY_STRING;
    else print_debug("Instruction SAY_STRING not found\n");
  
// ----- say Instruction Say op1 -----
instruction = src_inst("say", OP_CHAR,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&SAY_CHAR;
    else print_debug("Instruction SAY_CHAR not found\n");
  
// ----- exit Instruction Exit -----
instruction = src_inst("exit", OP_NONE,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&EXIT;
    else print_debug("Instruction EXIT not found\n");
  
// ----- exit Instruction Exit op1 -----
instruction = src_inst("exit", OP_REG,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&EXIT_REG;
    else print_debug("Instruction EXIT_REG not found\n");
  
// ----- exit Instruction Exit op1 -----
instruction = src_inst("exit", OP_INT,OP_NONE,OP_NONE);
if (instruction) address_map[instruction->opcode] = &&EXIT_INT;
    else print_debug("Instruction EXIT_INT not found\n");
  
