/*---------------------------------------------------------------------*/
/*                                                                     */
/* The CREXX Project                                                   */
/* DES in Rexx Using the CREXX/plugin interface                        */
/*                                                                     */
/* Purpose of this program:                                            */
/* Verification of the Des algorithm using the example in Price&Davies */
/* 'Security for Computer Networks', ISBN 0 471 92137 8, p. 64         */
/* functions RxDesEncrypt and RxDesDecrypt from the rxdes plugin       */
/*                                                                     */
/*---------------------------------------------------------------------*/

options levelb  /* This is a rexx level b program */

import rxfnsb   /* Import the crexx level B functions */
import rxdes    /* Import the rxdes plugin functions  */

/* Note that the input and output to the des functions are in hex strings */

Plaintext = "0000000000000000"
key =       "08192A3B4C5D6E7F"

say 'Plaintext is  ' Plaintext
say 'Key is        ' Key

say 'Encrypting ...'
                                           
Ciphertext = Encrypt(key,Plaintext)
Ciphertext = Encrypt("A key",Plaintext)

say 'Ciphertext is ' Ciphertext

say 'Decrypting ...'

DecryptedText = Decrypt(key,Ciphertext)

if (DecryptedText = Plaintext) then say 'Decrypted Ciphertext compares +.'
