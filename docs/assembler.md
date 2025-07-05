# REXX Assembler

## Comments

    /* Block Comment */

    * Line Comment

## Globals

### File Registers 

    .globals={int}

Defines {int} global variable g0 ... gn. These can be used
within any procedure in the file. 

### Global Registers

Any global register marked as exposed is available to any file
which also has the corresponding exposed index/name.

#### File 1

    .globals=2            * 2 Global Registers
    g0 .expose=namespace.var_name   * 

#### File 2

    .globals=3            * 3 Global Registers
    g2 .expose=namespace.var_name   * 

In this case file 1 g0 is mapped to file 2 g2 under the 
index/name of "namespace.var_name".

### Global Procedures

#### File 1

    * The ".locals" shows the procedure is defined in here
    proc() .locals=3 .expose=namespace.proc
    ret

#### File 2    

    * No ".locals" here! Showing that the procedure is only being declared 
    rproc() .expose=namespace.proc

    main() .locals=3 
    call rproc
    ret

In this case main() in File 2, calls rproc() which is globally provided
under the index/name of "namespace.proc". In File 1, proc() is exposed under
this name and hence called from File 1. 

Note: that "namespace" hints at the use of namespaces as part of exposed names.
Also, as shown names can be mapped - they don't have to be the same in the source 
and in the target.
