# Tools {#tools}

## **Compiler** {#compiler}

Usage   : rxc \[options\] source\_file

Options :

  \-h              Prints help message

  \-c              Prints Copyright & License Details

  \-v              Prints Version

  \-l location     Working Location (directory)

  \-i import       Locations to import file \- ";" delimited list

  \-o output\_file  REXX Assembler Output File

  \-n              No Optimising

## **Assembler** {#assembler}

Usage   : rxas \[options\] source\_file

Options :

  \-h              Help Message

  \-c              Copyright & License Details

  \-v              Version

  \-a              Architecture Details

  \-i              Print Instructions

  \-d              Debug/Verbose Mode

  \-l location     Working Location (directory)

  \-o output\_file  Binary Output File

  \-n              No Optimising

Note that the rxbin files can be simply concatenated together to form a library archive. 

## **Disassembler** {#disassembler}

Usage   : rxdas \[options\] binary\_file

Options :

  \-h              Help message

  \-c              Copyright & licence details

  \-v              Version

  \-p              all constant Pool

  \-l location     working Location (directory)

  \-o output\_file  Output file (default is stdout)

## **Debugger** {#debugger}

Usage   : rxvm \[options\] binary\_file \[binary\_file\_2 ...\] \-a args ... 

Options :

  \-h              Prints help message

  \-c              Prints Copyright & License Details

  \-l location     Working Location (directory)

  \-v              Prints Version

*CURRENT STATUS: This is a PoC version \- may not function correctly*

## **Packager** {#packager}

This takes .rxbin crexx binary files and converts them to a c data array. These can then be linked (using the native c toolchain) to rxvml vm library (see below) to form a standalone executable file.

*CURRENT STATUS: The generated exe's are quite big (not too bad) however the use of the upx exe compressor mitigates this*

Usage   : rxcpack \[options\] input\_file\_1 input\_file\_2 ... input\_file\_n

                           (.rxbin is appended to input file names)

Options :

  \-h              Help Message

  \-c              Copyright & License Details

  \-v              Version

  \-o output\_file  Binary Output File (.c is appended \- default is input\_file\_1.c)

**Virtual Machine**

The base vm is rxvm

Usage   : rxvm \[options\] binary\_file \[binary\_file\_2 ...\] \-a args ... 

Options :

  \-h              Prints help message

  \-c              Prints Copyright & License Details

  \-l location     Working Location (directory)

  \-v              Prints Version

This is the default interpreter that uses “threaded” instruction dispatching (this should not be confused with multi-threaded). 

Typically when running a rexx program with rxvm you should include library.rxbin which is the consolidated standard library of built in functions.

Other interpreters are:

* rxbvm \- equivalent to rxvm but using classic bytecode instruction dispatching  
* rxvme / rxvbme \- (b means bytecode version) interpreter linked with the cREXX stdlib (so that library.rxbin is not needed)  
* rxvml / rxbvml \- (b means bytecode version) interpreter library used to support standalone native exe generated from cREXX source. This linked (using the native c toolchain) to a program converted to C using the packager above, creates a standalone executable.

## **Helper Scripts** {#helper-scripts}

In addition to these there are scripts to automate the build flow (details tbc).

# 

# General Syntax {#general-syntax}

A REXX program consists of a series of statements, each of which is terminated by a semicolon (;) or line-end. The REXX source program is coded in utf-8. 

**Line continuation**

A comma located at the conclusion of a line (external to a string) results in the continuation of that line. This allows long statements to be broken across multiple lines for readability. 

**Labels**

Labels can be used to identify specific statements in a REXX program. Labels are followed by a colon (:) and must be placed in the first column of a line. For example:

**Comments**

Comments are used to provide additional information about a REXX program. Comments are ignored by the REXX interpreter and can be placed anywhere in a program.

# 

# Language Level and Options {#language-level-and-options}

All crexx programs can start with an “options’ instruction, which can include the language level (e.g. ‘levelb). If a crexx program does not start with an ‘options’ instruction or if the options do not include a language level then Level C (classic REXX) is assumed.

*CURRENT STATUS: Level C is not implemented. Also it may be that the default language level will be changed to Level G.*

The supported comment format can also be specified on the options.

*         Dash '--' line comments. Options: dashcomments  or nodashcomments (default)  
*         Slash '//' line comments. Options: slashcomments  or noslashcomments (default)  
*         Hash '\#' line comments. Options: hashcomments (default) or nohashcomments

The \# comment allows you to add the Posix first line shebang, e.g. 

\#\!/usr/local/crexx/rexx.sh

*EXAMPLE:*

options levelb slashcomments
