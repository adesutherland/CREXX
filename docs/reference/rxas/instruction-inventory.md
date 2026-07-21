# RXAS Instruction Inventory

This inventory is for coverage validation. Every mnemonic from `rxas -i` should appear exactly once with its current primary skeleton section.

- Unique mnemonics: 375
- Opcode/form rows: 582

## Section Counts

| Section | Mnemonics |
| --- | ---: |
| [Program Control And Calls](instructions/01-program-control.md) | 30 |
| [Data Movement And Conversion](instructions/02-data-movement-and-conversion.md) | 7 |
| [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md) | 44 |
| [Floating Point](instructions/04-floating-point.md) | 24 |
| [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md) | 46 |
| [Strings And Characters](instructions/06-strings-and-characters.md) | 34 |
| [Binary Memory](instructions/07-binary-memory.md) | 43 |
| [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md) | 32 |
| [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md) | 44 |
| [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md) | 19 |
| [Metadata And Introspection](instructions/11-metadata-and-introspection.md) | 23 |
| [Large And Fused Instructions](instructions/12-large-instructions.md) | 29 |
| [Uncategorized Review](instructions/99-uncategorized-review.md) | 0 |

## Mnemonic Index

| Mnemonic | Forms | Primary section |
| --- | ---: | --- |
| `acopy` | 1 | [Data Movement And Conversion](instructions/02-data-movement-and-conversion.md#acopy) |
| `and` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#and) |
| `append` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#append) |
| `appendchar` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#appendchar) |
| `arr2redir` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#arr2redir) |
| `assertinitialized` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#assertinitialized) |
| `asserttype` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#asserttype) |
| `bappend` | 1 | [Binary Memory](instructions/07-binary-memory.md#bappend) |
| `bcf` | 2 | [Program Control And Calls](instructions/01-program-control.md#bcf) |
| `bcheckrange` | 1 | [Binary Memory](instructions/07-binary-memory.md#bcheckrange) |
| `bclear` | 1 | [Binary Memory](instructions/07-binary-memory.md#bclear) |
| `bcmpb` | 4 | [Binary Memory](instructions/07-binary-memory.md#bcmpb) |
| `bcmps` | 4 | [Binary Memory](instructions/07-binary-memory.md#bcmps) |
| `bconcat` | 1 | [Binary Memory](instructions/07-binary-memory.md#bconcat) |
| `bcopy` | 3 | [Binary Memory](instructions/07-binary-memory.md#bcopy) |
| `bct` | 2 | [Program Control And Calls](instructions/01-program-control.md#bct) |
| `bctnm` | 2 | [Program Control And Calls](instructions/01-program-control.md#bctnm) |
| `bctp` | 1 | [Program Control And Calls](instructions/01-program-control.md#bctp) |
| `beq` | 2 | [Program Control And Calls](instructions/01-program-control.md#beq) |
| `bfill` | 1 | [Binary Memory](instructions/07-binary-memory.md#bfill) |
| `bge` | 2 | [Program Control And Calls](instructions/01-program-control.md#bge) |
| `bgetf32` | 2 | [Binary Memory](instructions/07-binary-memory.md#bgetf32) |
| `bgetf64` | 2 | [Binary Memory](instructions/07-binary-memory.md#bgetf64) |
| `bgeti16` | 2 | [Binary Memory](instructions/07-binary-memory.md#bgeti16) |
| `bgeti32` | 2 | [Binary Memory](instructions/07-binary-memory.md#bgeti32) |
| `bgeti64` | 2 | [Binary Memory](instructions/07-binary-memory.md#bgeti64) |
| `bgeti8` | 2 | [Binary Memory](instructions/07-binary-memory.md#bgeti8) |
| `bgets` | 2 | [Binary Memory](instructions/07-binary-memory.md#bgets) |
| `bgetu16` | 2 | [Binary Memory](instructions/07-binary-memory.md#bgetu16) |
| `bgetu32` | 2 | [Binary Memory](instructions/07-binary-memory.md#bgetu32) |
| `bgetu8` | 2 | [Binary Memory](instructions/07-binary-memory.md#bgetu8) |
| `bgt` | 2 | [Program Control And Calls](instructions/01-program-control.md#bgt) |
| `bineq` | 2 | [Binary Memory](instructions/07-binary-memory.md#bineq-and-binne) |
| `binne` | 2 | [Binary Memory](instructions/07-binary-memory.md#bineq-and-binne) |
| `bintos` | 1 | [Binary Memory](instructions/07-binary-memory.md#bintos) |
| `ble` | 2 | [Program Control And Calls](instructions/01-program-control.md#ble) |
| `blen` | 2 | [Binary Memory](instructions/07-binary-memory.md#blen) |
| `blt` | 2 | [Program Control And Calls](instructions/01-program-control.md#blt) |
| `bmemmove` | 1 | [Binary Memory](instructions/07-binary-memory.md#bmemmove) |
| `bmove` | 1 | [Binary Memory](instructions/07-binary-memory.md#bmove) |
| `bne` | 2 | [Program Control And Calls](instructions/01-program-control.md#bne) |
| `bpoff` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#bpoff) |
| `bpon` | 2 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#bpon) |
| `br` | 1 | [Program Control And Calls](instructions/01-program-control.md#br) |
| `bresize` | 1 | [Binary Memory](instructions/07-binary-memory.md#bresize) |
| `brf` | 1 | [Program Control And Calls](instructions/01-program-control.md#brf) |
| `brt` | 1 | [Program Control And Calls](instructions/01-program-control.md#brt) |
| `brtf` | 1 | [Program Control And Calls](instructions/01-program-control.md#brtf) |
| `brtpandt` | 1 | [Program Control And Calls](instructions/01-program-control.md#brtpandt) |
| `brtpt` | 1 | [Program Control And Calls](instructions/01-program-control.md#brtpt) |
| `bsetf32` | 1 | [Binary Memory](instructions/07-binary-memory.md#bsetf32) |
| `bsetf64` | 1 | [Binary Memory](instructions/07-binary-memory.md#bsetf64) |
| `bseti16` | 1 | [Binary Memory](instructions/07-binary-memory.md#bseti16) |
| `bseti32` | 1 | [Binary Memory](instructions/07-binary-memory.md#bseti32) |
| `bseti64` | 1 | [Binary Memory](instructions/07-binary-memory.md#bseti64) |
| `bseti8` | 1 | [Binary Memory](instructions/07-binary-memory.md#bseti8) |
| `bsets` | 2 | [Binary Memory](instructions/07-binary-memory.md#bsets) |
| `bsetu16` | 1 | [Binary Memory](instructions/07-binary-memory.md#bsetu16) |
| `bsetu32` | 1 | [Binary Memory](instructions/07-binary-memory.md#bsetu32) |
| `bsetu8` | 1 | [Binary Memory](instructions/07-binary-memory.md#bsetu8) |
| `bslice` | 1 | [Binary Memory](instructions/07-binary-memory.md#bslice) |
| `btod` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#btod) |
| `btof` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#btof) |
| `btoi` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#btoi) |
| `btos` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#btos) |
| `bupdate` | 1 | [Binary Memory](instructions/07-binary-memory.md#bupdate) |
| `call` | 3 | [Program Control And Calls](instructions/01-program-control.md#call) |
| `call1` | 1 | [Program Control And Calls](instructions/01-program-control.md#call1) |
| `call2` | 1 | [Program Control And Calls](instructions/01-program-control.md#call2) |
| `call3` | 1 | [Program Control And Calls](instructions/01-program-control.md#call3) |
| `call4` | 1 | [Program Control And Calls](instructions/01-program-control.md#call4) |
| `cnop` | 2 | [Program Control And Calls](instructions/01-program-control.md#cnop) |
| `concat` | 3 | [Strings And Characters](instructions/06-strings-and-characters.md#concat) |
| `concchar` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#concchar) |
| `copy` | 1 | [Data Movement And Conversion](instructions/02-data-movement-and-conversion.md#copy) |
| `dadd` | 2 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dadd) |
| `dcall` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dcall) |
| `dcopy` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dcopy) |
| `ddiv` | 3 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#ddiv) |
| `dec` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dec) |
| `dec0` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dec0) |
| `dec1` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dec1) |
| `dec2` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dec2) |
| `decplnm` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#decplnm) |
| `delattrs` | 4 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#delattrs) |
| `delattrs1` | 4 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#delattrs1) |
| `deq` | 2 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#deq) |
| `deqbr` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#deqbr) |
| `deref` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#deref) |
| `dextr` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dextr) |
| `dgt` | 3 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dgt) |
| `dgtbr` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dgtbr) |
| `dgte` | 3 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dgte) |
| `didiv` | 3 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#didiv) |
| `dlt` | 3 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dlt) |
| `dltbr` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dltbr) |
| `dlte` | 3 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dlte) |
| `dmod` | 3 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dmod) |
| `dmult` | 2 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dmult) |
| `dne` | 2 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dne) |
| `dpow` | 3 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dpow) |
| `dropchar` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dropchar) |
| `dsex` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dsex) |
| `dsub` | 3 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dsub) |
| `dtob` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dtob) |
| `dtof` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dtof) |
| `dtoi` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dtoi) |
| `dtos` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#dtos) |
| `endlife` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#endlife) |
| `erase` | 1 | [Data Movement And Conversion](instructions/02-data-movement-and-conversion.md#erase) |
| `exit` | 3 | [Program Control And Calls](instructions/01-program-control.md#exit) |
| `fadd` | 2 | [Floating Point](instructions/04-floating-point.md#fadd) |
| `fclearerr` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#fclearerr) |
| `fclose` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#fclose) |
| `fcopy` | 1 | [Floating Point](instructions/04-floating-point.md#fcopy) |
| `fdiv` | 3 | [Floating Point](instructions/04-floating-point.md#fdiv) |
| `fdivsub` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#fdivsub) |
| `feof` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#feof) |
| `feq` | 2 | [Floating Point](instructions/04-floating-point.md#feq) |
| `ferror` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#ferror) |
| `fextr` | 1 | [Floating Point](instructions/04-floating-point.md#fextr) |
| `fflush` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#fflush) |
| `fformat` | 1 | [Floating Point](instructions/04-floating-point.md#fformat) |
| `fgt` | 3 | [Floating Point](instructions/04-floating-point.md#fgt) |
| `fgtbr` | 1 | [Floating Point](instructions/04-floating-point.md#fgtbr) |
| `fgte` | 3 | [Floating Point](instructions/04-floating-point.md#fgte) |
| `fidiv` | 3 | [Floating Point](instructions/04-floating-point.md#fidiv) |
| `flt` | 3 | [Floating Point](instructions/04-floating-point.md#flt) |
| `fltbr` | 1 | [Floating Point](instructions/04-floating-point.md#fltbr) |
| `flte` | 3 | [Floating Point](instructions/04-floating-point.md#flte) |
| `fmod` | 3 | [Floating Point](instructions/04-floating-point.md#fmod) |
| `fmult` | 2 | [Floating Point](instructions/04-floating-point.md#fmult) |
| `fmulticopy` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#fmulticopy) |
| `fndblnk` | 1 | [Floating Point](instructions/04-floating-point.md#fndblnk) |
| `fndnblnk` | 1 | [Floating Point](instructions/04-floating-point.md#fndnblnk) |
| `fne` | 2 | [Floating Point](instructions/04-floating-point.md#fne) |
| `fopen` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#fopen) |
| `fpow` | 3 | [Floating Point](instructions/04-floating-point.md#fpow) |
| `freadb` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#freadb) |
| `freadbyte` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#freadbyte) |
| `freadcdpt` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#freadcdpt) |
| `freadline` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#freadline) |
| `fsex` | 1 | [Floating Point](instructions/04-floating-point.md#fsex) |
| `fsub` | 3 | [Floating Point](instructions/04-floating-point.md#fsub) |
| `ftob` | 1 | [Floating Point](instructions/04-floating-point.md#ftob) |
| `ftod` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#ftod) |
| `ftoi` | 1 | [Floating Point](instructions/04-floating-point.md#ftoi) |
| `ftos` | 1 | [Floating Point](instructions/04-floating-point.md#ftos) |
| `fwrite` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#fwrite) |
| `fwriteb` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#fwriteb) |
| `fwritebyte` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#fwritebyte) |
| `fwritecdpt` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#fwritecdpt) |
| `getabufs` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#getabufs) |
| `getandtp` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#getandtp) |
| `getattrs` | 2 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#getattrs) |
| `getbinpos` | 1 | [Binary Memory](instructions/07-binary-memory.md#getbinpos) |
| `getbyte` | 1 | [Binary Memory](instructions/07-binary-memory.md#getbyte) |
| `getenv` | 2 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#getenv) |
| `getnumcas` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#getnumcas) |
| `getnumdgts` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#getnumdgts) |
| `getnumfrm` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#getnumfrm) |
| `getnumfuz` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#getnumfuz) |
| `getnumstd` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#getnumstd) |
| `getstrpos` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#getstrpos) |
| `gettp` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#gettp) |
| `hexchar` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#hexchar) |
| `iadd` | 2 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#iadd) |
| `iand` | 2 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#iand) |
| `ichkrng` | 5 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#ichkrng) |
| `icopy` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#icopy) |
| `idiv` | 3 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#idiv) |
| `ieq` | 2 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#ieq) |
| `igetunlink` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#igetunlink) |
| `igt` | 3 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#igt) |
| `igtbr` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#igtbr) |
| `igte` | 3 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#igte) |
| `ilt` | 3 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#ilt) |
| `iltbr` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#iltbr) |
| `ilte` | 3 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#ilte) |
| `iloadsetunlink` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#iloadsetunlink) |
| `iloadsetunlinkn` | 2 | [Large And Fused Instructions](instructions/12-large-instructions.md#iloadsetunlinkn) |
| `imod` | 3 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#imod) |
| `imult` | 2 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#imult) |
| `inc` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#inc) |
| `inc0` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#inc0) |
| `inc1` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#inc1) |
| `inc2` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#inc2) |
| `ine` | 2 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#ine) |
| `inot` | 2 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#inot) |
| `insattrs` | 4 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#insattrs) |
| `insattrs1` | 4 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#insattrs1) |
| `ior` | 2 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#ior) |
| `ipow` | 3 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#ipow) |
| `irand` | 2 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#irand) |
| `isex` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#isex) |
| `isetattr1` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#isetattr1) |
| `isetunlink` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#isetunlink) |
| `isetunlinkn` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#isetunlinkn) |
| `ishl` | 2 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#ishl) |
| `ishr` | 2 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#ishr) |
| `isinitialized` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#isinitialized) |
| `istype` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#istype) |
| `isub` | 3 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#isub) |
| `itob` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#itob) |
| `itod` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#itod) |
| `itof` | 2 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#itof) |
| `itos` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#itos) |
| `ixor` | 2 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#ixor) |
| `jumpb` | 1 | [Program Control And Calls](instructions/01-program-control.md#jumpb) |
| `jumpbs` | 1 | [Program Control And Calls](instructions/01-program-control.md#jumpbs) |
| `jumpi` | 1 | [Program Control And Calls](instructions/01-program-control.md#jumpi) |
| `jumpn` | 1 | [Program Control And Calls](instructions/01-program-control.md#jumpn) |
| `jumpr` | 1 | [Program Control And Calls](instructions/01-program-control.md#jumpr) |
| `jumps` | 1 | [Program Control And Calls](instructions/01-program-control.md#jumps) |
| `link` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#link) |
| `linkarg` | 2 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#linkarg) |
| `linkattr` | 2 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#linkattr) |
| `linkattr1` | 2 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#linkattr1) |
| `linkref` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#linkref) |
| `linksetattrslinkadd` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#linksetattrslinkadd) |
| `linktoattr` | 2 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#linktoattr) |
| `linktoattr1` | 2 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#linktoattr1) |
| `load` | 8 | [Data Movement And Conversion](instructions/02-data-movement-and-conversion.md#load) |
| `loadsettp` | 3 | [Data Movement And Conversion](instructions/02-data-movement-and-conversion.md#loadsettp) |
| `loadsettp2` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#loadsettp2) |
| `loadsettpswap` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#loadsettpswap) |
| `metadecodeinst` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#metadecodeinst) |
| `metalinkpreg` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#metalinkpreg) |
| `metaloadcalleraddr` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#metaloadcalleraddr) |
| `metaloaddata` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#metaloaddata) |
| `metaloadedeprocs` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#metaloadedeprocs) |
| `metaloadedmodules` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#metaloadedmodules) |
| `metaloadedprocs` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#metaloadedprocs) |
| `metaloadfoperand` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#metaloadfoperand) |
| `metaloadinst` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#metaloadinst) |
| `metaloadioperand` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#metaloadioperand) |
| `metaloadmodule` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#metaloadmodule) |
| `metaloadpoperand` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#metaloadpoperand) |
| `metaloadsoperand` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#metaloadsoperand) |
| `minattrs` | 4 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#minattrs) |
| `minlinkattr1` | 2 | [Large And Fused Instructions](instructions/12-large-instructions.md#minlinkattr1) |
| `mkref` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#mkref) |
| `move` | 1 | [Data Movement And Conversion](instructions/02-data-movement-and-conversion.md#move) |
| `mtime` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#mtime) |
| `not` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#not) |
| `null` | 1 | [Data Movement And Conversion](instructions/02-data-movement-and-conversion.md#null) |
| `nulln` | 3 | [Large And Fused Instructions](instructions/12-large-instructions.md#nulln) |
| `nullredir` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#nullredir) |
| `numeng` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#numeng) |
| `numsci` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#numsci) |
| `or` | 1 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#or) |
| `padstr` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#padstr) |
| `parseplan` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#parseplan) |
| `parsepos2` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#parsepos2) |
| `parsewords3` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#parsewords3) |
| `parsewords3d` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#parsewords3d) |
| `poschar` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#poschar) |
| `readline` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#readline) |
| `redir2arr` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#redir2arr) |
| `redir2str` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#redir2str) |
| `refvalid` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#refvalid) |
| `req` | 3 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#req) |
| `ret` | 5 | [Program Control And Calls](instructions/01-program-control.md#ret) |
| `rgt` | 3 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#rgt) |
| `rgte` | 3 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#rgte) |
| `rlt` | 3 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#rlt) |
| `rlte` | 3 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#rlte) |
| `rne` | 3 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#rne) |
| `rseq` | 2 | [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md#rseq) |
| `rxhash` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#rxhash) |
| `rxvers` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#rxvers) |
| `sappend` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#sappend) |
| `say` | 5 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#say) |
| `sayx` | 2 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sayx) |
| `sconcat` | 3 | [Strings And Characters](instructions/06-strings-and-characters.md#sconcat) |
| `scopy` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#scopy) |
| `seq` | 2 | [Strings And Characters](instructions/06-strings-and-characters.md#seq) |
| `setattrs` | 4 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#setattrs) |
| `setbinpos` | 1 | [Binary Memory](instructions/07-binary-memory.md#setbinpos) |
| `setbyte` | 1 | [Binary Memory](instructions/07-binary-memory.md#setbyte) |
| `setlinkattr1` | 2 | [Large And Fused Instructions](instructions/12-large-instructions.md#setlinkattr1) |
| `setlinkiload` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#setlinkiload) |
| `setnumcas` | 2 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#setnumcas) |
| `setnumdgts` | 2 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#setnumdgts) |
| `setnumfrm` | 2 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#setnumfrm) |
| `setnumfuz` | 2 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#setnumfuz) |
| `setnumstd` | 2 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#setnumstd) |
| `setobjtype` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#setobjtype) |
| `setobjuninit` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#setobjuninit) |
| `setortp` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#setortp) |
| `setref` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#setref) |
| `setstrpos` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#setstrpos) |
| `settp` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#settp) |
| `settpcall` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#settpcall) |
| `settpmask` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#settpmask) |
| `settpswap` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#settpswap) |
| `settpswapcall` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#settpswapcall) |
| `settpswapsettpswap` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#settpswapsettpswap) |
| `sget` | 1 | [Binary Memory](instructions/07-binary-memory.md#sget) |
| `sgt` | 3 | [Strings And Characters](instructions/06-strings-and-characters.md#sgt) |
| `sgte` | 3 | [Strings And Characters](instructions/06-strings-and-characters.md#sgte) |
| `sigbr` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#sigbr) |
| `sigbrv` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#sigbrv) |
| `sigcall` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#sigcall) |
| `sigcalla` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#sigcalla) |
| `sigcallbr` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#sigcallbr) |
| `sighalt` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#sighalt) |
| `sigignore` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#sigignore) |
| `signal` | 5 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#signal) |
| `signalf` | 2 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#signalf) |
| `signalt` | 2 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#signalt) |
| `sigpop` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#sigpop) |
| `sigpush` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#sigpush) |
| `sigret` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#sigret) |
| `sigshalt` | 1 | [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md#sigshalt) |
| `slt` | 3 | [Strings And Characters](instructions/06-strings-and-characters.md#slt) |
| `slte` | 3 | [Strings And Characters](instructions/06-strings-and-characters.md#slte) |
| `sne` | 2 | [Strings And Characters](instructions/06-strings-and-characters.md#sne) |
| `sockaccept` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockaccept) |
| `sockbind` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockbind) |
| `sockblocking` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockblocking) |
| `sockclose` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockclose) |
| `sockconnect` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockconnect) |
| `sockconnecttls` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockconnecttls) |
| `sockerror` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockerror) |
| `sockkeepalive` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockkeepalive) |
| `socklisten` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#socklisten) |
| `socklocal` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#socklocal) |
| `socknew` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#socknew) |
| `socknodelay` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#socknodelay) |
| `sockpeer` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockpeer) |
| `sockpending` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockpending) |
| `sockrecv` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockrecv) |
| `sockrecvb` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockrecvb) |
| `socksend` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#socksend) |
| `socksendb` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#socksendb) |
| `sockshutdown` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockshutdown) |
| `sockstarttls` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockstarttls) |
| `sockstatus` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#sockstatus) |
| `socktimeout` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#socktimeout) |
| `spawn` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#spawn) |
| `srcfprocsel` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#srcfprocsel) |
| `srcmethodsel` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#srcmethodsel) |
| `stob` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#stob) |
| `stobin` | 1 | [Binary Memory](instructions/07-binary-memory.md#stobin) |
| `stod` | 1 | [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md#stod) |
| `stof` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#stof) |
| `stoi` | 2 | [Strings And Characters](instructions/06-strings-and-characters.md#stoi) |
| `str2redir` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#str2redir) |
| `strchar` | 2 | [Strings And Characters](instructions/06-strings-and-characters.md#strchar) |
| `strlen` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#strlen) |
| `strlower` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#strlower) |
| `strpos` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#strpos) |
| `strupper` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#strupper) |
| `substcut` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#substcut) |
| `substr` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#substr) |
| `substring` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#substring) |
| `swap` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#swap) |
| `swapcall` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#swapcall) |
| `swapn` | 3 | [Large And Fused Instructions](instructions/12-large-instructions.md#swapn) |
| `swapsettp` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#swapsettp) |
| `swapsettpswap` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#swapsettpswap) |
| `time` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#time) |
| `transchar` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#transchar) |
| `triml` | 2 | [Strings And Characters](instructions/06-strings-and-characters.md#triml) |
| `trimr` | 2 | [Strings And Characters](instructions/06-strings-and-characters.md#trimr) |
| `trunc` | 1 | [Strings And Characters](instructions/06-strings-and-characters.md#trunc) |
| `typeof` | 1 | [Metadata And Introspection](instructions/11-metadata-and-introspection.md#typeof) |
| `unlink` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#unlink) |
| `unlinkattr` | 2 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#unlinkattr) |
| `unlinkattr1` | 2 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#unlinkattr1) |
| `unlinkbr` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#unlinkbr) |
| `unlinkn` | 1 | [Large And Fused Instructions](instructions/12-large-instructions.md#unlinkn) |
| `unref` | 1 | [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md#unref) |
| `xtime` | 1 | [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md#xtime) |
