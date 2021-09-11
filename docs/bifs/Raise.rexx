 /***********************************************************************
 *.  Raise
 ***********************************************************************/
 Raise:
/* These 40.nn messages always include the built-in name as an insert.*/
    call !Raise 'SYNTAX', arg(1), !Bif, arg(2), arg(3), arg(4)
    /* !Raise does not return. */
