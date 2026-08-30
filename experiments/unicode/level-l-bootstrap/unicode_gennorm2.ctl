* Link the generated Level L scanner and hand-written Unicode rule parser.
INPUT gennorm2.rxbin
INPUT unicode_gennorm2.rxbin
INCLUDE unicode_gennorm2
PRESERVE INLINE
OUTPUT unicode_gennorm2_linked
