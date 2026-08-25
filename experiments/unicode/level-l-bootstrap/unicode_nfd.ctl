* Link the generated rule lexer, typed parser, and portable NFD proof.
INPUT gennorm2.rxbin
INPUT unicode_gennorm2.rxbin
INPUT unicode_nfd.rxbin
INCLUDE unicode_nfd
PRESERVE INLINE
OUTPUT unicode_nfd_linked
