* Link canonical per-instance and shared-constant Unicode runtimes.
INPUT unicode_gennorm2.rxbin
INPUT unicode_data.rxbin
INPUT unicode_normprops.rxbin
INPUT unicode_d.rxbin
INPUT unicode_casefold.rxbin
INPUT unicode_normalization.rxbin
INPUT level_l_unicode_nfd.rxbin
INCLUDE unicode_data
INCLUDE unicode_normprops
INCLUDE unicode_d
INCLUDE unicode_casefold
INCLUDE unicode_normalization
INCLUDE level_l_unicode_nfd
PRESERVE INLINE
OUTPUT unicode_common_compare_linked
