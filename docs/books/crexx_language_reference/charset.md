# The character set of \crexx{} programs

There are two sets of characters of relevance to \crexx{} programming: the set of characters in which programs are written, and the set of the characters that form the data manipulated by those programs. The latter includes all Unicode codepoints, represented as UTF-8 or other interchange formats -- but of course all sorts of non-character data can be processed by these programs.

The former set, from which \crexx{} programs are built, is a chosen subset of UTF-8 which is larger than the set of uppercase- and lowercase characters, sometimes including some special characters, in ASCII or EBCDIC, which was supported by some older \rexx{} implementations, but clearly puts limits on the number of usable characters for programs. The criterium here is TODO YES WHICH CRITERIUM

For use on older platforms like VM370CE the use of EBCDIC for program text is still possible.
