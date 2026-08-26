* Link the UnicodeData parser and prepared Unicode D-form implementation.
INPUT unicode_gennorm2.rxbin
INPUT unicode_data.rxbin
INPUT unicode_d.rxbin
INCLUDE unicode_d
PRESERVE INLINE
OUTPUT unicode_d_linked
