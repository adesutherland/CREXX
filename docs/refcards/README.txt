Reference Cards

These are a throwback to the IBM "Green Card" (of various colours) which contained a summary of the instruction set, and the recently re-published "REX" reference card. The RXASM card is the first, and is in the rxasm_refcard.pdf file. It is, for the part of the instruction set, automatically generated from rxas -i.

As other tools made it hard to automate or approximate the historycal layout, there are some requirements for building these.

- XelateX (a variant of LateX that uses system fonts 'as installed' on your platform). A modern TeXLive or MacTeX install will suffice, except for:
- The TeX Gyre Termes font, which is an open source Times Roman, but better: as Mike Cowlishaw prefers Rexx to be written in smallcaps, you will need this version, because the fonts distributed with Windows and macOS are old and miss the small caps font variant
- (for now) NetRexx, because the output of rxas -i is massaged with two CMS Pipelines (will be converted to cRexx as soon as it is able to do this)

The included makefile.refcard comprises everything that is needed.

Nota bene: Because of extensive trickery to have the 'tabularx' environment work in a multicolumn setup, on some systems the build stalls with a 'superfluous \align' message - this should be ignored by entering 'q' at that point; the card in the pdf file will build successfully.

This is not the final form. Of course, the card will be adapted for every release of the RXAX assembler, also, some of the traditional character set and hex/dec tables will be added.

Next, the CREXX card will be attempted. All input is welcome.
