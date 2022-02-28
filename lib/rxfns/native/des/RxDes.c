/*------------------------------------------------------------------*/
/*                                                                  */
/* Name: RxDes.C                                                    */
/*                                                                  */
/* Copyright René Jansen june 1st 1993                              */
/*                                                                  */
/* Function: Rexx external functions RxDesEncrypt and RxDesDecrypt  */
/*                                                                  */
/* dependencies/calls: desbase.dll  REXXSAA                         */
/*                                                                  */
/* nmake statements:                                                */
/*                                                                  */
/* RxDes.dll: RxDes.c RxDes.mak desbase.dll                         */
/*       icc /Gmd4e- /O /Wall RxDes.c RxDes.def desbase.lib         */
/*------------------------------------------------------------------*/

#define INCL_NOPM
#include <os2.h>
#define INCL_REXXSAA
#include <rexxsaa.h>
#include <string.h>
#include <stdio.h>

RexxFunctionHandler RxDesEncrypt;
RexxFunctionHandler RxDesDecrypt;

ULONG RxDesEncrypt(
     PSZ       Name,
     ULONG     Argc,
     RXSTRING  Argv[],
     PSZ       Queuename,
     PRXSTRING Retstr)

{
  CHAR   desout[9];
  CHAR   des_in[9];

  if ( Argc != 2)
     return 40L;

  desout[8] = '\0';
  des_in[8] = '\0';

  memcpy(des_in,Argv[0].strptr,8);

  desinit(des_in);

  memcpy(des_in,Argv[1].strptr,8);

  endes(des_in,desout);

  memcpy(Retstr->strptr, desout, 8);

  Retstr->strlength = 8;

  return 0L;

}
ULONG RxDesDecrypt(
     PSZ       Name,
     ULONG     Argc,
     RXSTRING  Argv[],
     PSZ       Queuename,
     PRXSTRING Retstr)

{
  CHAR   desout[9];
  CHAR   des_in[9];

  if ( Argc != 2)
     return 40L;

  desout[8] = '\0';
  des_in[8] = '\0';

  memcpy(des_in,Argv[0].strptr,8);

  desinit(des_in);

  memcpy(des_in,Argv[1].strptr,8);

  dedes(des_in,desout);

  memcpy(Retstr->strptr, desout, 8);

  Retstr->strlength = 8;

  return 0L;

}

