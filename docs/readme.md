# The cRexx documents[^caveat]

[^caveat]: Do not expect to build the documents yourself at this point in time. Some work is needed for that. The state of the documents is pre-alpha.

## Contents
This directory tree contains the sources for the cRexx documents. The toolchain for this is documented elsewhere.
These documents are planned:

- The cRexx Language Reference
- The cRexx Application Programming Guide
- The cRexx Reference Card
- The cRexx VM Specification

## Goals

The goal here is to have complete and accessible documentation, which can be used online and in printable form, as PDF or physical book. For this reason the toolchain leans on two technologies, which are Markdown and Latex. The intention is that the Latex part is, for the most part, invisible to the document writer.

Another important goal is that the documents are correct and up to date with respect to the level of cRexx they describe. In principle all examples need to executable, and runnable during the document building stage so that they can produce verifiable output, which is to be included in the text. Here an example of this principle is documented for the cRexx VM Specification publication is shown:

In the directory ```docs/books/crexx_vm_spec/examples``` there are examples of instructions, in small programs which are named after the instruction signatures[^signatures]. This looks like:

``` shell
  /Users/rvjansen/apps/crexx-f0049/docs/books/crexx_vm_spec/examples: (69 GiB available)
  drwxr-xr-x@ 31 rvjansen  staff   992 20 okt  2023 .
  drwxr-xr-x@ 47 rvjansen  staff  1504 29 aug 16:10 ..
  -rw-r--r--@  1 rvjansen  staff   266  3 jul  2023 bgeIDREGINT.rxas
  -rw-r--r--   1 rvjansen  staff   514 20 okt  2023 bgeIDREGINT.rxbin
  -rw-r--r--@  1 rvjansen  staff   281  3 jul  2023 bgeIDREGREG.rxas
  -rw-r--r--   1 rvjansen  staff   538 20 okt  2023 bgeIDREGREG.rxbin
  -rw-r--r--@  1 rvjansen  staff   299  3 jul  2023 bgtIDREGINT.rxas
  -rw-r--r--   1 rvjansen  staff   674 20 okt  2023 bgtIDREGINT.rxbin
  -rw-r--r--@  1 rvjansen  staff   263  3 jul  2023 bltIDREGINT.rxas
  -rw-r--r--   1 rvjansen  staff   514 20 okt  2023 bltIDREGINT.rxbin
  -rw-r--r--@  1 rvjansen  staff   280  3 jul  2023 bltIDREGREG.rxas
  -rw-r--r--   1 rvjansen  staff   538 20 okt  2023 bltIDREGREG.rxbin
  -rw-r--r--@  1 rvjansen  staff   321  3 jul  2023 brID.rxas
  -rw-r--r--   1 rvjansen  staff   692 20 okt  2023 brID.rxbin
  -rw-r--r--@  1 rvjansen  staff   319  3 jul  2023 concatREGREGREG.rxas
  -rw-r--r--   1 rvjansen  staff   706 20 okt  2023 concatREGREGREG.rxbin
  -rw-r--r--@  1 rvjansen  staff   293  3 jul  2023 copies.rxas
  -rw-r--r--@  1 rvjansen  staff   279  3 jul  2023 dec0no.rxas
  -rw-r--r--   1 rvjansen  staff   320 20 okt  2023 dec0no.rxbin
  -rw-r--r--@  1 rvjansen  staff   275  3 jul  2023 dec1no.rxas
  -rw-r--r--   1 rvjansen  staff   320 20 okt  2023 dec1no.rxbin
  -rw-r--r--@  1 rvjansen  staff   274  3 jul  2023 dec2no.rxas
  -rw-r--r--   1 rvjansen  staff   320 20 okt  2023 dec2no.rxbin
  -rw-r--r--@  1 rvjansen  staff   277  3 jul  2023 decREG.rxas
  -rw-r--r--@  1 rvjansen  staff   328 20 okt  2023 decREG.rxbin
  -rw-r--r--@  1 rvjansen  staff   381  3 jul  2023 fsexREG.rxas
  -rw-r--r--   1 rvjansen  staff   378 20 okt  2023 fsexREG.rxbin
  -rw-r--r--@  1 rvjansen  staff   314  3 jul  2023 igtREGREGREG.rxas
  -rw-r--r--   1 rvjansen  staff   700 20 okt  2023 igtREGREGREG.rxbin
  -rw-r--r--@  1 rvjansen  staff   370  3 jul  2023 isexREG.rxas
  -rw-r--r--   1 rvjansen  staff   378 20 okt  2023 isexREG.rxbin
```
The program should be short, illustrate the core of the functionality and have limited output; this because the program and its output will end up in the book.

[^signatures]: A signature is here defined as the name of the instruction plus its arguments; as one instruction can have multiple forms, as an execution of ```rxas -i``` illustrates, a valid filename is constructed from the name and its arguments, these being Strings, Registers or IDs.
