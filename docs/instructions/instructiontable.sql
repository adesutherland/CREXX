drop table instructions;
CREATE TABLE location
( instruction        integer primary key,
  opcode             char(6),
  mnemonic           varchar(50),
  operands	     varchar(50),
  description	     varchar(80),
  text		     varchar(1000),
  category	     integer
);
drop table category;
CREATE TABLE category
( category           integer primary key,
  name               varchar(80),
  description	     varchar(80),
);
insert into category VALUES(1,'Fixed Point Arithmetic','Instructions that work on integer numeric types');
insert into category VALUES(2,'Floating Point Arithmetic','Instructions that work on floating point numeric types');
insert into category VALUES(3,'Logical Operations','Instructions that perform logic');
insert into category VALUES(4,'Branching','Instructions that perform jumps and implement decisions');
insert into category VALUES(5,'I/O operations','Instructions that implement input and output operations');
insert into category VALUES(6,'Time Instructions','Instructions that work with date and time');
insert into category VALUES(7,'Meta Instructions','Instructions that work on program structure metadata');
insert into category VALUES(8,'Breakpoint Instructions','Instructions that aid in debugging programs');
COMMIT;
