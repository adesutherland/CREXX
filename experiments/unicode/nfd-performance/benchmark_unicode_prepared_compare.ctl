* Link the original scalar NFD and the prepared four-form implementations.
INPUT gennorm2.rxbin
INPUT unicode_gennorm2.rxbin
INPUT unicode_nfd.rxbin
INPUT unicode_data.rxbin
INPUT unicode_normprops.rxbin
INPUT unicode_d.rxbin
INPUT level_l_unicode_nfd.rxbin
INCLUDE unicode_nfd
INCLUDE unicode_d
INCLUDE level_l_unicode_nfd
PRESERVE INLINE
OUTPUT unicode_prepared_compare_linked
