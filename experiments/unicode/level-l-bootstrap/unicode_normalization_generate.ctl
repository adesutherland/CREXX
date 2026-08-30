* Link the prepared Unicode compiler used to freeze the runtime constant.
INPUT unicode_gennorm2.rxbin
INPUT unicode_data.rxbin
INPUT unicode_normprops.rxbin
INPUT unicode_d.rxbin
INPUT unicode_casefold.rxbin
INCLUDE unicode_d
INCLUDE unicode_casefold
PRESERVE INLINE
OUTPUT unicode_normalization_generate_linked
