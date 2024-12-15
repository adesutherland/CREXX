/*---------------------------------------------------------------------*/
/* DNB C SOURCE MODULE HEADER                         SP/MVS/DB/DC/PC  */
/*---------------------------------------------------------------------*/
/*                                                                     */
/* MODULE NAME= DESBASE.C                                              */
/*                                                                     */
/* DESCRIPTIVE NAME= Des file encryption functions package             */
/*                                                                     */
/*      COPYRIGHT = COPYRIGHT RJ @ DE NEDERLANDSCHE BANK N.V. 1993     */
/*                                                                     */
/*      STATUS = VERSION 1 RELEASE 0, LEVEL 0                          */
/*                                                                     */
/*      PROGRAMMER NAME = René Jansen (adapted from dr.Dobbs version)  */
/*                                                                     */
/* FUNCTION=  encrypt and decrypt using the DES algorithm              */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/* NOTES=                                                              */
/*    DEPENDENCIES=  no                                                */
/*    RESTRICTIONS=                                                    */
/*    DISPATCHING =                                                    */
/*                                                                     */
/*                                                                     */
/* MODULE TYPE=  ANSI C                                                */
/*                                                                     */
/*    PROCESSOR= IBM C/2 (DOS + 16b OS/2) IBM C++ (OS/2 32b) GNU (UNIX)*/
/*                                                                     */
/*    ATTRIBUTES=                                                      */
/*                                                                     */
/* INPUT=                                                              */
/*  SYMBOLIC LABEL/NAME=                                               */
/*  DESCRIPTION=                                                       */
/*                                                                     */
/* OUTPUT=                                                             */
/*  SYMBOLIC LABEL/NAME=                                               */
/*  DESCRIPTION=                                                       */
/*                                                                     */
/*                                                                     */
/*    OS functions  =  no platform dependant functions, strictly ANSI  */
/*                     compliant.                                      */
/*                                                                     */
/* MACROS=                                                             */
/*                                                                     */
/* HISTORY=                                                            */
/*                                                                     */
/* 930228 v1.00   BY RJ                      TESTED BY: RJ             */
/* CHNG REQ NO:                                                        */
/*        CHANGED BY                         TESTED BY:                */
/*                                                                     */
/*---------------------------------------------------------------------*/
/* desbase.c: the Data Encryption Standard DLL source
 *     The permutation is defined by its effect on each of the 16 nibbles
 *     of the 64-bit input.  For each nibble we give an 8-byte bit array
 *     that has the bits in the input nibble distributed correctly.  The
 *     complete permutation involves ORing the 16 sets of 8 bytes designated
 *     by the 16 input nibbles.  Uses 16*16*8 = 2K bytes of storage for
 *     each 64-bit permutation.  32-bit permutations (P) and expansion (E)
 *     are done similarly, but using bytes instead of nibbles.
 *
 * The compressions are pre-computed in 12-bit chunks, combining 2 of the
 *     6->4 bit compressions.
 * The key schedule is also precomputed.
 *
 * This version contains just the DES engine for use in a 32-bit DLL
 * for use in various applications (René Jansen @ DNB).
 *
 *  it can be called in this way:
 *
 *  char key[9];
 *  char invoer[9] = "TEXTTEXT";
 *  char uitvoer[9] = "";
 *  strcpy(key,"TESTTEST");
 *  desinit(key);
 *  endes(invoer,uitvoer);
 *  dedes(invoer,uitvoer);
 *
*/

#ifndef DESBASE_H
#define DESBASE_H

/* initialize all des arrays */
void desinit(char *key);

/* encrypt 64-bit inblock  */
void endes(char* inblock, char* outblock);

/* decrypt 64-bit inblock  */
void dedes(char* inblock, char* outblock);

#endif // DESBASE_H
