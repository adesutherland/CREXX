* Link the UnicodeData/property parsers and prepared normalization implementation.
INPUT unicode_gennorm2.rxbin
INPUT unicode_data.rxbin
INPUT unicode_normprops.rxbin
INPUT unicode_d.rxbin
INPUT level_l_unicode_nfd.rxbin
INCLUDE unicode_d
INCLUDE level_l_unicode_nfd
PRESERVE INLINE
OUTPUT unicode_d_linked
