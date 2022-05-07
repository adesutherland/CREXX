/*---------------------------------------------------------------------*/
/*                                                                     */
/* rexx SAA OS/2                                                       */
/* DES in Rexx Using the External Function Interface                   */
/*                                                                     */
/* Purpose of this exec:                                               */
/* Verification of the Des algorithm using the example in Price&Davies */
/* 'Security for Computer Networks', ISBN 0 471 92137 8, p. 64         */
/* functions RxDesEncrypt and RxDesDecrypt from RxDes.dll              */
/* these functions call the standard desbase.dll                       */
/*                                                                     */
/*---------------------------------------------------------------------*/

 CALL ON ERROR NAME Label

 rc = RXFUNCADD('RxDesEncrypt','RxDes','RxDesEncrypt')

 rc = RXFUNCADD('RxDesDecrypt','RxDes','RxDesDecrypt')
                      
 Plaintext = X2C(0000000000000000)

 key = X2C(08192A3B4C5D6E7F)

 say 'Plaintext is  ' C2X(Plaintext)

 say 'Key is        ' C2X(Key) 

 say 'Encrypting ...'
                                           
 Ciphertext = RxDesEncrypt(key,Plaintext)

 say 'Ciphertext is ' C2X(Ciphertext) 

 say 'Decrypting ...'

 PlainText = RxDesDecrypt(key,Ciphertext)

 if (PlainText = Plaintext) then say 'Decrypted Ciphertext compares +.' 

label:

 rc = RXFUNCDROP('RxDesEncrypt')
 rc = RXFUNCDROP('RxDesDecrypt')
