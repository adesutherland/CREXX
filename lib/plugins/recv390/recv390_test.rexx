/* RXPP */
/* ----------------------------------------------------------------------
 * PRE Compiled on 4 Nov 2025  at 17:57:41
 * ----------------------------------------------------------------------
 */
options levelb
import rxfnsb
import recv390
/* ##cflags def nset  3buf  parse def */
##  array=.string[]
/* ------------------------------------------------------
 * extract a file from the CBT library
 * ------------------------------------------------------
 */
xmitFile = 'C:/Temp/CBT112/FILE112.XMI'   ## XMI file to work with
/* ------------------------------------------------------
 * Create an Array containing the PDS directory
 * ------------------------------------------------------
 */
pdsdir=.string[]
  call xmitdirlist xmitFile,pdsdir
/* now report it */
  say copies('-',80)
  say 'Directory List of XMI File '
  say copies('-',80)
  do i=1 to pdsdir[0]
     say '  'pdsdir[i]
  end
say pdsdir[0]' Entries in Directory'
say copies('.',80)
say 'End of Directory List'
say copies('.',80)
say ' '
/* ------------------------------------------------------
 * Receiving the full PDS from the XMI file
 * ------------------------------------------------------
 */
say time('l')' Unpack 'xmitFile
rc = xmitunpack(xmitFile)
say time('l')' Unpack finished with RC=' rc
/*  --------------- will be next -------------- */
rc = xmitextract(xmitFile,"$$$#DATE")
call xmitcleanup
