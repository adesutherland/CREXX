/*
 * crexx RXPP
 * CREXX Pre Compiler
 */
options levelb
import rxfnsb
/* ##set printgen nnest S*/
/* rxpp: say SQUARE(DOUBLE(4)) */
say 2*4*2*4
/* Macro definitions */
/* ##set prefix 1234 S*/
/* rxpp: log() */
 say time('l')' log record' ; say '1234 something'
/* ##set prefix 456 S*/
/* rxpp: log() */
 say time('l')' log record' ; say '456 something'
/* rxpp: list2stem(fruits, "apple", "banana", "cherry") */
fruits.1="apple"
fruits.2="banana"
fruits.3="cherry"
/* rxpp: say hi(fruits) */
say fruits.0
/* rxpp: liststem(fruits) */
do _indx=1 to fruits.0; say fruits._indx ; end
/* rxpp: say DOUBLE(9)             /* double 9              */ */
 say 2*9             /* double 9              */
/* rxpp: say SQUARE(DOUBLE(4))     /* square after doubling */ */
 say 2*4*2*4     /* square after doubling */
 fred=9
/* rxpp: say SQUARE(fred)          /* square fred */ */
 say fred*fred          /* square fred */
/* loop through fruit array */
/* rxpp: foreach(fruits,j) */
   do j=1 to fruits.0 
     say fruits.j
   end
/*    ##set prefix 789 S*/
/* rxpp: log() */
 say time('l')' log record' ; say '789 something'
/*  ##set prefix old S*/
/* rxpp: log() */
 say time('l')' log record' ; say 'old something'
/*  ##set prefix new S*/
/* rxpp: log() */
 say time('l')' log record' ; say 'new something'
/* rxpp: list2stem(fruits, "apple", "banana", "cherry") */
fruits.1="apple"
fruits.2="banana"
fruits.3="cherry"
/* rxpp: say hi(fruits) */
say fruits.0
/* rxpp: liststem(fruits) */
do _indx=1 to fruits.0; say fruits._indx ; end
/* rxpp: say DOUBLE(9)             /* double 9              */ */
 say 2*9             /* double 9              */
/* rxpp: say SQUARE(DOUBLE(4))     /* square after doubling */ */
 say 2*4*2*4     /* square after doubling */
 fred=9
/* rxpp: say SQUARE(fred)          /* square fred */ */
 say fred*fred          /* square fred */
/* loop through fruit array */
/* rxpp: foreach(fruits,j) */
   do j=1 to fruits.0 
     say fruits.j
   end
/* rxpp: log() */
 say time('l')' log record' ; say 'new something'
/* rxpp: log() */
 say time('l')' log record' ; say 'new something'
/* rxpp: log() */
 say time('l')' log record' ; say 'new something'