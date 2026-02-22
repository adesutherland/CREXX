/*---------------------------------------------------------------------*/
/*                                                                     */
/* The CREXX Project                                                   */
/* DES in Rexx Using the CREXX/plugin interface                        */
/*                                                                     */
/* Purpose of this program:                                            */
/* Test an invalid input to trigger a rexx signal error                */
/*---------------------------------------------------------------------*/

options levelb  /* This is a rexx level b program */

import rxfnsb   /* Import the crexx level B functions */
import rxdes    /* Import the rxdes plugin functions  */

/* Note that the input and output to the des functions are in hex strings */

Plaintext = "0000000000000000"
key =       "XXXXXXXXXXXXXXXX" /* Invalid hex key */

say 'Plaintext is  ' Plaintext
say 'Key is        ' Key

say 'Encrypting ...'
                                           
Ciphertext = Encrypt(key,Plaintext)

say 'Ciphertext is ' Ciphertext

say 'Decrypting ...'

DecryptedText = Decrypt(key,Ciphertext)

if (DecryptedText = Plaintext) then say 'Decrypted Ciphertext compares +.'
