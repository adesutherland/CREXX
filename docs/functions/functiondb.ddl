drop table if exists signature;
drop table if exists funcvariant;
drop table if exists variant;
drop table if exists name;

CREATE TABLE name
(
  name               varchar(80) primary key,
  description	     varchar(80)
);

CREATE TABLE variant
( variant            integer primary key,
  name               varchar(80) not null,
  version            varchar(40) not null,	
  description	     varchar(80)
);
insert into variant VALUES(1,'TRL1','1','The Rexx Language version 1 (Blue)');
insert into variant VALUES(2,'TRL2','1','The Rexx Language version 2 (Red)');
insert into variant VALUES(3,'SAA','1','System Application Architecture Procedures Language');
insert into variant VALUES(4,'ooRexx','5','RexxLA Open Object Rexx');
insert into variant VALUES(5,'NetRexx','5','RexxLA NetRexx');
insert into variant VALUES(6,'Regina','3.9','Regina');
insert into variant VALUES(7,'BREXX','x','BREXX');
insert into variant VALUES(8,'CMS Rexx','x','CREXX');
insert into variant VALUES(9,'TSO Rexx','x','CREXX');


CREATE TABLE funcvariant
( funcvar            integer primary key,
  name               varchar(50),
  variant            integer not null,
  package            varchar(50),
  description	     varchar(80),
  foreign key (variant) references variant(variant),
  foreign key (name) references name(name)
);

CREATE TABLE signature
( funcvar     integer primary key,
  signature   varchar(1000),
  foreign key (funcvar) references funcvariant(funcvar)
);


