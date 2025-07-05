# Namespace

In Rexx programming, the ```namespace``` instruction plays a crucial role in organizing and structuring modules within a program. It allows you to define a unique identifier for a module, enabling multiple modules to coexist within the same program while maintaining their own identity and scope.

The <!--index-->namespace instruction is used after the ```options``` instruction in a \crexx{} program. By specifying a namespace, you define the logical group or category to which a particular <!--index-->module belongs. This helps in organizing related modules together and facilitates their identification and usage from other modules within the program.

As an example, the built-in-functions package, has a namespace of ```rxfnsb```, where he ```b``` suffix indicates the language level. The built-in function <!--index-->insert (written in \rexx{}) looks like this:

```rexx <!--insert.rexx-->
/* rexx */
options levelb
namespace rxfnsb expose insert

/* insert(insstr,string,position,length,pad) 
inserts string into existing string at certain position and length */

insert: procedure = .string
arg insstr = .string, string = .string, position = -1, len = -1, pad = " "

slen=length(string)
  if pad='' then pad=' '

  if len>0 then insstr=substr(insstr,1,len,pad) /* was there an insert string length?      */
                                                /* format insert string to requested length*/
  else if len=0 then insstr=""
  if position<=0 then return insstr||string

  if position>slen then string=substr(string,1,position,pad)  /* is position>string length, extend string */

  str1=substr(string,1,position)             /* extract first part of string       */
  if position>=slen then str2=''             /* nothing remains of string          */
  else str2=substr(string,position+1)        /*    else create str2                */
 return str1||insstr||str2                   /* return newly constructed string    */
```

The ```namespace``` statement on line 3 identifies this function as part of the ```rxfnsb``` namespace, which, in \crexx{} level B, needs to be imported into a program which makes use of it. 

```rexx <!--insertexample.rexx-->
options levelb
import rxfnsb

say insert("abc","def",2,1)
```

<!--splice--insertexample.rexx-->


One of the key benefits of using namespaces is that it allows you to control the exposure of procedures, members, and globals from one module to another. When you define a namespace, you can specify which elements of that module will be visible and accessible to other modules. This enhances <!--index-->modularity and <!--index-->encapsulation by limiting the <!--index-->scope of variables and procedures to specific namespaces, preventing potential conflicts and promoting code reusability.

If a module does not explicitly include a namespace instruction, \crexx{} automatically assigns it to a default namespace which is derived from the file name without the ```.rexx``` file type. 

By leveraging namespaces, \crexx{} programmers can create modular and well-organized programs, enhancing code readability, maintainability, and reusability. Namespaces promote a structured approach to program and library design, allowing developers to group related functionality together and control the visibility of elements across different modules.
