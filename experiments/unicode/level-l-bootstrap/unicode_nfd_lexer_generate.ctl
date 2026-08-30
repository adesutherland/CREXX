* Link the prepared Unicode compiler used to generate the NFD scanner rules.
INPUT unicode_gennorm2.rxbin
INPUT unicode_data.rxbin
INPUT unicode_normprops.rxbin
INPUT unicode_d.rxbin
INCLUDE unicode_d
PRESERVE INLINE
OUTPUT unicode_nfd_lexer_generate_linked
