/* ------------------------------------------------------------------------------------
 *  SCONCAT_REG_REG_REG  String Concat (op1=op2||op3)              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
SCONCAT_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    DEBUG("TRACE - SCONCAT_REG_REG_REG");
    DEBUG("SCONCAT_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    v2 = REG_OP(2);
    REG_TEST(!v2);
    v3 = REG_OP(3);
    REG_TEST(!v3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  SCONCAT_REG_REG_STRING  String Concat (op1=op2||op3)              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
SCONCAT_REG_REG_STRING: // label not yet defined
  CALC_DISPATCH(3);
    DEBUG("TRACE - SCONCAT_REG_REG_STRING");
    DEBUG("SCONCAT_REG_REG_STRING not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    v2 = REG_OP(2);
    REG_TEST(!v2);
    s3 = CONSTSTRING_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  SCONCAT_REG_STRING_REG  String Concat (op1=op2||op3)              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
SCONCAT_REG_STRING_REG: // label not yet defined
  CALC_DISPATCH(3);
    DEBUG("TRACE - SCONCAT_REG_STRING_REG");
    DEBUG("SCONCAT_REG_STRING_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    s2 = CONSTSTRING_OP(2);
    v3 = REG_OP(3);
    REG_TEST(!v3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  CONCATS_REG_REG_REG  Convert and Concat to String (op1=op2||op3)              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
CONCATS_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    DEBUG("TRACE - CONCATS_REG_REG_REG");
    DEBUG("CONCATS_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    v2 = REG_OP(2);
    REG_TEST(!v2);
    v3 = REG_OP(3);
    REG_TEST(!v3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  CONCATS_REG_REG_STRING  Convert and Concat to String (op1=op2||op3)              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
CONCATS_REG_REG_STRING: // label not yet defined
  CALC_DISPATCH(3);
    DEBUG("TRACE - CONCATS_REG_REG_STRING");
    DEBUG("CONCATS_REG_REG_STRING not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    v2 = REG_OP(2);
    REG_TEST(!v2);
    s3 = CONSTSTRING_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  CONCATS_REG_STRING_REG  Convert and Concat to String (op1=op2||op3)              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
CONCATS_REG_STRING_REG: // label not yet defined
  CALC_DISPATCH(3);
    DEBUG("TRACE - CONCATS_REG_STRING_REG");
    DEBUG("CONCATS_REG_STRING_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    s2 = CONSTSTRING_OP(2);
    v3 = REG_OP(3);
    REG_TEST(!v3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  STRLEN_REG_REG  String Length op1 = length(op2)              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
STRLEN_REG_REG: // label not yet defined
  CALC_DISPATCH(2);
    DEBUG("TRACE - STRLEN_REG_REG");
    DEBUG("STRLEN_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    v2 = REG_OP(2);
    REG_TEST(!v2);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  STR2CHAR_REG_REG_REG  String to Char op1 = op2[op3]              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
STR2CHAR_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    DEBUG("TRACE - STR2CHAR_REG_REG_REG");
    DEBUG("STR2CHAR_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    v2 = REG_OP(2);
    REG_TEST(!v2);
    v3 = REG_OP(3);
    REG_TEST(!v3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  CPRIME_REG  Char Prime op1              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
CPRIME_REG: // label not yet defined
  CALC_DISPATCH(1);
    DEBUG("TRACE - CPRIME_REG");
    DEBUG("CPRIME_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  MAP_REG_REG  Map op1 to var name in op2              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
MAP_REG_REG: // label not yet defined
  CALC_DISPATCH(2);
    DEBUG("TRACE - MAP_REG_REG");
    DEBUG("MAP_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    v2 = REG_OP(2);
    REG_TEST(!v2);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  MAP_REG_STRING  Map op1 to var name op2              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
MAP_REG_STRING: // label not yet defined
  CALC_DISPATCH(2);
    DEBUG("TRACE - MAP_REG_STRING");
    DEBUG("MAP_REG_STRING not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    s2 = CONSTSTRING_OP(2);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  PMAP_REG_REG  Map op1 to parent var name in op2              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
PMAP_REG_REG: // label not yet defined
  CALC_DISPATCH(2);
    DEBUG("TRACE - PMAP_REG_REG");
    DEBUG("PMAP_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    v2 = REG_OP(2);
    REG_TEST(!v2);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  PMAP_REG_STRING  Map op1 to parent var name op2              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
PMAP_REG_STRING: // label not yet defined
  CALC_DISPATCH(2);
    DEBUG("TRACE - PMAP_REG_STRING");
    DEBUG("PMAP_REG_STRING not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    s2 = CONSTSTRING_OP(2);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  GMAP_REG_REG  Map op1 to global var name in op2              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
GMAP_REG_REG: // label not yet defined
  CALC_DISPATCH(2);
    DEBUG("TRACE - GMAP_REG_REG");
    DEBUG("GMAP_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    v2 = REG_OP(2);
    REG_TEST(!v2);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  GMAP_REG_STRING  Map op1 to global var name op2              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
GMAP_REG_STRING: // label not yet defined
  CALC_DISPATCH(2);
    DEBUG("TRACE - GMAP_REG_STRING");
    DEBUG("GMAP_REG_STRING not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    s2 = CONSTSTRING_OP(2);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  NSMAP_REG_REG_REG  Map op1 to namespace in op2 var name in op3              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
NSMAP_REG_REG_REG: // label not yet defined
  CALC_DISPATCH(3);
    DEBUG("TRACE - NSMAP_REG_REG_REG");
    DEBUG("NSMAP_REG_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    v2 = REG_OP(2);
    REG_TEST(!v2);
    v3 = REG_OP(3);
    REG_TEST(!v3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  NSMAP_REG_REG_STRING  Map op1 to namespace in op2 var name op3              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
NSMAP_REG_REG_STRING: // label not yet defined
  CALC_DISPATCH(3);
    DEBUG("TRACE - NSMAP_REG_REG_STRING");
    DEBUG("NSMAP_REG_REG_STRING not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    v2 = REG_OP(2);
    REG_TEST(!v2);
    s3 = CONSTSTRING_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  NSMAP_REG_STRING_STRING  Map op1 to namespace op2 var name op3              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
NSMAP_REG_STRING_STRING: // label not yet defined
  CALC_DISPATCH(3);
    DEBUG("TRACE - NSMAP_REG_STRING_STRING");
    DEBUG("NSMAP_REG_STRING_STRING not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    s2 = CONSTSTRING_OP(2);
    s3 = CONSTSTRING_OP(3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  NSMAP_REG_STRING_REG  Map op1 to namespace op2 var name in op3              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
NSMAP_REG_STRING_REG: // label not yet defined
  CALC_DISPATCH(3);
    DEBUG("TRACE - NSMAP_REG_STRING_REG");
    DEBUG("NSMAP_REG_STRING_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    s2 = CONSTSTRING_OP(2);
    v3 = REG_OP(3);
    REG_TEST(!v3);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  UNMAP_REG  Unmap op1              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
UNMAP_REG: // label not yet defined
  CALC_DISPATCH(1);
    DEBUG("TRACE - UNMAP_REG");
    DEBUG("UNMAP_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  RET_CHAR  Return op1              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
RET_CHAR: // label not yet defined
  CALC_DISPATCH(1);
    DEBUG("TRACE - RET_CHAR");
    DEBUG("RET_CHAR not yet defined\n");
    goto SIGNAL;
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  LINK_REG_REG  Link op2 to op1              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
LINK_REG_REG: // label not yet defined
  CALC_DISPATCH(2);
    DEBUG("TRACE - LINK_REG_REG");
    DEBUG("LINK_REG_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    v2 = REG_OP(2);
    REG_TEST(!v2);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  UNLINK_REG  Unlink op1              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
UNLINK_REG: // label not yet defined
  CALC_DISPATCH(1);
    DEBUG("TRACE - UNLINK_REG");
    DEBUG("UNLINK_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  NULL_REG  Null op1              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
NULL_REG: // label not yet defined
  CALC_DISPATCH(1);
    DEBUG("TRACE - NULL_REG");
    DEBUG("NULL_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  LOAD_REG_CHAR  Load op1 with op2              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
LOAD_REG_CHAR: // label not yet defined
  CALC_DISPATCH(2);
    DEBUG("TRACE - LOAD_REG_CHAR");
    DEBUG("LOAD_REG_CHAR not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  EXIT_REG  Exit op1              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
EXIT_REG: // label not yet defined
  CALC_DISPATCH(1);
    DEBUG("TRACE - EXIT_REG");
    DEBUG("EXIT_REG not yet defined\n");
    goto SIGNAL;
    v1 = REG_OP(1);
    REG_TEST(!v1);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
/* ------------------------------------------------------------------------------------
 *  EXIT_INT  Exit op1              pej 17 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
EXIT_INT: // label not yet defined
  CALC_DISPATCH(1);
    DEBUG("TRACE - EXIT_INT");
    DEBUG("EXIT_INT not yet defined\n");
    goto SIGNAL;
    i1 = INT_OP(1);
    // Add your coding 
    /* REG_OP(1)="?????"; */
  DISPATCH;
  
