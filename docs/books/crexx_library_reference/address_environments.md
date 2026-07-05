# Address Environments

> A command is a simple mechanism for sending a message to some functional unit
> external to a REXX program. It is usually a request for some service or action, and
> consists of a single character string. Many operating systems and other programs
> have a command language interface of this nature.[^trl1]


The *crexxsaa* facility enables the implementation of addressing environments. These can adress functionality using external programs, in the style of Classic Rexx *subcommand handlers*, or implement macro functionality for applications.

Some Address Environments, like `address crexx`, `address shell`, and others, are part of the language core. Other environments can be best classified as libraries; regardless, all are packaged as libraries. In this part of the library reference these environments are discussed and documented. A good example is the `address sqlite` environment, which delivers an embedded SQL interface to SQLite and can serve as an example of delivering this for other database engines.

Other documentation addressing environments are:

- SQLite: a demonstration of Embedded SQL[^embedded];
- CMS: a demonstration of sending commands to an environment which simulates a VM/CMS host;
- THE: The Hessling Editor, a close approximation of the VM/CMS XEDIT editor[^the];

[^trl1]: <!--cite-->[cowlishaw1985rexx]

[^embedded]: Embedded SQL is a style of database access from programs which included blocks of SQL directly into the source code of programs, so without functions calls. This is the standard approach for COBOL, PL/I and some other languages.

[^the]: including the capability to execute cRexx macro's in THE editor.
