/* RXPP */
/* ----------------------------------------------------------------------
 * PRE Compiled on 1 Nov 2025  at 13:49:32
 * ----------------------------------------------------------------------
 */
options levelb
import rxfnsb
import recv390
/* ##cflags def nset  3buf  parse def */
##  array=.string[]
/* REXX: unpack_xmit.rex */
xmitFile = 'C:/Temp/CBT112/FILE112.XMI'
say time('l')' Unpack 'xmitFile
rc = xmitunpack(xmitFile)
say time('l')' Unpack finished with RC=' rc
call xmitcleanup
say 'Cleanup complete.'
