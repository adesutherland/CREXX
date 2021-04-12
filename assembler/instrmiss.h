/* ------------------------------------------------------------------------------------
 *  ADDI_REG_REG_REG  Convert and Add to Integer (op1=op2+op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
ADDI_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("ADDI_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  ADDI_REG_REG_INT  Convert and Add to Integer (op1=op2+op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
ADDI_REG_REG_INT: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("ADDI_REG_REG_INT not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    i3 = INT_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  SUBI_REG_REG_REG  Convert and Subtract to Integer (op1=op2-op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
SUBI_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("SUBI_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  SUBI_REG_REG_INT  Convert and Subtract to Integer (op1=op2-op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
SUBI_REG_REG_INT: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("SUBI_REG_REG_INT not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    i3 = INT_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  MULTI_REG_REG_REG  Convert and Multiply to Integer (op1=op2*op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
MULTI_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("MULTI_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  MULTI_REG_REG_INT  Convert and Multiply to Integer (op1=op2*op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
MULTI_REG_REG_INT: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("MULTI_REG_REG_INT not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    i3 = INT_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  DIVI_REG_REG_REG  Convert and Divide to Integer (op1=op2/op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
DIVI_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("DIVI_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  DIVI_REG_REG_INT  Convert and Divide to Integer (op1=op2/op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
DIVI_REG_REG_INT: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("DIVI_REG_REG_INT not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    i3 = INT_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  ADDF_REG_REG_REG  Convert and Add to Float (op1=op2+op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
ADDF_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("ADDF_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  ADDF_REG_REG_FLOAT  Convert and Add to Float (op1=op2+op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
ADDF_REG_REG_FLOAT: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("ADDF_REG_REG_FLOAT not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    f3 = FLOAT_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  SUBF_REG_REG_REG  Convert and Subtract to Float (op1=op2-op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
SUBF_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("SUBF_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  SUBF_REG_REG_FLOAT  Convert and Subtract to Float (op1=op2-op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
SUBF_REG_REG_FLOAT: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("SUBF_REG_REG_FLOAT not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    f3 = FLOAT_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  SUBF_REG_FLOAT_REG  Convert and Subtract to Float (op1=op2-op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
SUBF_REG_FLOAT_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("SUBF_REG_FLOAT_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    f2 = FLOAT_OP(2);
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  MULTF_REG_REG_REG  Convert and Multiply to Float (op1=op2*op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
MULTF_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("MULTF_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  MULTF_REG_REG_FLOAT  Convert and Multiply to Float (op1=op2*op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
MULTF_REG_REG_FLOAT: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("MULTF_REG_REG_FLOAT not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    f3 = FLOAT_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  DIVF_REG_REG_REG  Convert and Divide to Float (op1=op2/op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
DIVF_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("DIVF_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  DIVF_REG_REG_FLOAT  Convert and Divide to Float (op1=op2/op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
DIVF_REG_REG_FLOAT: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("DIVF_REG_REG_FLOAT not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    f3 = FLOAT_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  DIVF_REG_FLOAT_REG  Convert and Divide to Float (op1=op2/op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
DIVF_REG_FLOAT_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("DIVF_REG_FLOAT_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    f2 = FLOAT_OP(2);
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  SCONCAT_REG_REG_REG  String Concat (op1=op2||op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
SCONCAT_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("SCONCAT_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  SCONCAT_REG_REG_STRING  String Concat (op1=op2||op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
SCONCAT_REG_REG_STRING: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("SCONCAT_REG_REG_STRING not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    s3 = CONSTSTRING_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  SCONCAT_REG_STRING_REG  String Concat (op1=op2||op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
SCONCAT_REG_STRING_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("SCONCAT_REG_STRING_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    s2 = CONSTSTRING_OP(2);
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  CONCATS_REG_REG_REG  Convert and Concat to String (op1=op2||op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
CONCATS_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("CONCATS_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  CONCATS_REG_REG_STRING  Convert and Concat to String (op1=op2||op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
CONCATS_REG_REG_STRING: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("CONCATS_REG_REG_STRING not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    s3 = CONSTSTRING_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  CONCATS_REG_STRING_REG  Convert and Concat to String (op1=op2||op3)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
CONCATS_REG_STRING_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("CONCATS_REG_STRING_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    s2 = CONSTSTRING_OP(2);
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  STRLEN_REG_REG  String Length op1 = length(op2)              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
STRLEN_REG_REG: // label not yet defined
  CALC_DISPATCH(2);
    print_debug("STRLEN_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  STR2CHAR_REG_REG_REG  String to Char op1 = op2[op3]              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
STR2CHAR_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("STR2CHAR_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;

/* ------------------------------------------------------------------------------------
 *  IPRIME_REG  Int Prime op1              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
IPRIME_REG: // label not yet defined
  CALC_DISPATCH(1);
    print_debug("IPRIME_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  FPRIME_REG  Float Prime op1              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
FPRIME_REG: // label not yet defined
  CALC_DISPATCH(1);
    print_debug("FPRIME_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  SPRIME_REG  String Prime op1              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
SPRIME_REG: // label not yet defined
  CALC_DISPATCH(1);
    print_debug("SPRIME_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  CPRIME_REG  Char Prime op1              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
CPRIME_REG: // label not yet defined
  CALC_DISPATCH(1);
    print_debug("CPRIME_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  MAP_REG_REG  Map op1 to var name in op2              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
MAP_REG_REG: // label not yet defined
  CALC_DISPATCH(2);
    print_debug("MAP_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  MAP_REG_STRING  Map op1 to var name op2              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
MAP_REG_STRING: // label not yet defined
  CALC_DISPATCH(2);
    print_debug("MAP_REG_STRING not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    s2 = CONSTSTRING_OP(2);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  PMAP_REG_REG  Map op1 to parent var name in op2              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
PMAP_REG_REG: // label not yet defined
  CALC_DISPATCH(2);
    print_debug("PMAP_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  PMAP_REG_STRING  Map op1 to parent var name op2              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
PMAP_REG_STRING: // label not yet defined
  CALC_DISPATCH(2);
    print_debug("PMAP_REG_STRING not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    s2 = CONSTSTRING_OP(2);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  GMAP_REG_REG  Map op1 to global var name in op2              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
GMAP_REG_REG: // label not yet defined
  CALC_DISPATCH(2);
    print_debug("GMAP_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  GMAP_REG_STRING  Map op1 to global var name op2              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
GMAP_REG_STRING: // label not yet defined
  CALC_DISPATCH(2);
    print_debug("GMAP_REG_STRING not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    s2 = CONSTSTRING_OP(2);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  NSMAP_REG_REG_REG  Map op1 to namespace in op2 var name in op3              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
NSMAP_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("NSMAP_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  NSMAP_REG_REG_STRING  Map op1 to namespace in op2 var name op3              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
NSMAP_REG_REG_STRING: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("NSMAP_REG_REG_STRING not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    s3 = CONSTSTRING_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  NSMAP_REG_STRING_STRING  Map op1 to namespace op2 var name op3              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
NSMAP_REG_STRING_STRING: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("NSMAP_REG_STRING_STRING not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    s2 = CONSTSTRING_OP(2);
    s3 = CONSTSTRING_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  NSMAP_REG_STRING_REG  Map op1 to namespace op2 var name in op3              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
NSMAP_REG_STRING_REG: // label not yet defined
  CALC_DISPATCH(3);
    print_debug("NSMAP_REG_STRING_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    s2 = CONSTSTRING_OP(2);
    v3 = REG_OP(3);
    if (!v3) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  UNMAP_REG  Unmap op1              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
UNMAP_REG: // label not yet defined
  CALC_DISPATCH(1);
    print_debug("UNMAP_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;

/* ------------------------------------------------------------------------------------
 *  RET_CHAR  Return op1              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
RET_CHAR: // label not yet defined
  CALC_DISPATCH(1);
    print_debug("RET_CHAR not yet defined\n");
    goto SIGNAL;
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  RET_STRING  Return op1              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
RET_STRING: // label not yet defined
  CALC_DISPATCH(1);
    print_debug("RET_STRING not yet defined\n");
    goto SIGNAL;
    s1 = CONSTSTRING_OP(1);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  LINK_REG_REG  Link op2 to op1              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
LINK_REG_REG: // label not yet defined
  CALC_DISPATCH(2);
    print_debug("LINK_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    v2 = REG_OP(2);
    if (!v2) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  UNLINK_REG  Unlink op1              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
UNLINK_REG: // label not yet defined
  CALC_DISPATCH(1);
    print_debug("UNLINK_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  NULL_REG  Null op1              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
NULL_REG: // label not yet defined
  CALC_DISPATCH(1);
    print_debug("NULL_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  LOAD_REG_CHAR  Load op1 with op2              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
LOAD_REG_CHAR: // label not yet defined
  CALC_DISPATCH(2);
    print_debug("LOAD_REG_CHAR not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  EXIT_REG  Exit op1              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
EXIT_REG: // label not yet defined
  CALC_DISPATCH(1);
    print_debug("EXIT_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    if (!v1) REG_NOT();
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  EXIT_INT  Exit op1              pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
EXIT_INT: // label not yet defined
  CALC_DISPATCH(1);
    print_debug("EXIT_INT not yet defined\n");
    goto SIGNAL;
    i1 = INT_OP(1);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
