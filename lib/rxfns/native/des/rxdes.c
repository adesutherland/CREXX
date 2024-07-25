/*------------------------------------------------------------------*/
/*                                                                  */
/* Name: rxdes.C                                                    */
/*                                                                  */
/* Copyright René Jansen june 1st 1993                              */
/*                                                                  */
/* Function: crexx external functions RxDesEncrypt and RxDesDecrypt */
/*                                                                  */
/* dependencies/calls: desbase.c CREXX/rxpa                     */
/*                                                                  */
/* Ported to CREXX by Adrian Sutherland 2024                        */
/*------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include "desbase.h"    // DES encryption/decryption functions

// Helper function to convert a hex digit to a decimal digit (0-15)
// Returns -1 if the character is not a valid hex digit
static int hex2dec(char c)
{
  if( c >= '0' && c <= '9')
    return c - '0';
  if( c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if( c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

// Helper function to convert a string of 8 hex bytes (e.g. '12AB34CD12AB34CD') to a binary string
// Returns -1 if the string is not a valid hex string
static int hex2bin(const char* hex, char* bin)
{
  int i;
  for( i = 0; i < 16; i+=2)
  {
    int hi = hex2dec(hex[i]);
    int lo = hex2dec(hex[i+1]);
    if( hi < 0 || lo < 0)
      return -1;
    bin[i/2] = (char)(hi * 16 + lo);
  }
  return 0;
}

// Helper function to a binary string of 8 bytes to a hex string of 16 bytes plus
// a null terminator (e.g. '12AB34CD12AB34CD')
static void bin2hex(const char* bin, char* hex)
{
  int i;
  for( i = 0; i < 8; i++)
  {
    sprintf(hex + i * 2, "%02X", (unsigned char)bin[i]);
  }
}

// Encrypt a string using DES - the first argument is the key, the second the data
PROCEDURE(RxDesEncrypt)
{
    char   des_out[8];
    char   des_in[8];
    char   key[8];
    char   result[17];

    if( NUM_ARGS != 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")

    if (strlen(GETSTRING(ARG(0))) != 16 || strlen(GETSTRING(ARG(1))) != 16)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Both arguments must be 16 hex digits")

    if( hex2bin(GETSTRING(ARG(0)), key) < 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Key is not valid hex")

    if( hex2bin(GETSTRING(ARG(1)), des_in) < 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Data is not valid hex")

    // Encrypt
    desinit(key);
    endes(des_in, des_out);

    // Set return and make sure the signal is reset/ok
    bin2hex(des_out, result);
    SETSTRING(RETURN, result);
    RESETSIGNAL
}

// Decrypt a string using DES - the first argument is the key, the second the data
PROCEDURE(RxDesDecrypt)
{
    char   des_out[8];
    char   des_in[8];
    char   key[8];
    char   result[17];

    if( NUM_ARGS != 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")

    if (strlen(GETSTRING(ARG(0))) != 16 || strlen(GETSTRING(ARG(1))) != 16)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Both arguments must be 16 hex digits")

    if( hex2bin(GETSTRING(ARG(0)), key) < 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Key is not valid hex")

    if( hex2bin(GETSTRING(ARG(1)), des_in) < 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Data is not valid hex")

    // Decrypt
    desinit(key);
    dedes(des_in, des_out);

    // Set return and make sure the signal is reset/ok
    bin2hex(des_out, result);
    SETSTRING(RETURN, result);
    RESETSIGNAL
}

// Functions to be provided to rexx - these are loaded either when the plugin is loaded (dynamic) or
// before main() is called (static)
LOADFUNCS
//      C Function__, REXX namespace & name, Option_, Return Type_, Arguments
ADDPROC(RxDesDecrypt, "rxdes.decrypt",       "b",     ".string",    "key=.string,data=.string");
ADDPROC(RxDesEncrypt, "rxdes.encrypt",       "b",     ".string",    "key=.string,data=.string");
ENDLOADFUNCS

// End of file