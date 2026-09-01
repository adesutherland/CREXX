/* Version-2 deterministic RFC 4648 Base64 workload for ooRexx.
 * The historical source remains unchanged. Decode maps byte codepoints by
 * arithmetic ranges instead of searching the alphabet for every digit.
 */
parse arg repetitions
if repetitions='' then repetitions=1
if repetitions<1 then call fail 'repetitions must be positive'

input=''
do offset=0 to 1023
  input=input || d2c((offset*31+7)//256)
end

encoded=''
decoded=''
do iteration=1 to repetitions
  encoded=base64Encode(input)
  decoded=base64Decode(encoded)
end

if length(encoded)<>1368 then call fail 'expected encoded length 1368, got' length(encoded)
if input\==decoded then call fail 'decoded bytes differ from input'
checksum=byteChecksum(decoded)
if checksum<>130560 then call fail 'expected checksum 130560, got' checksum

say 'benchmark=base64_roundtrip_v2 repetitions='repetitions 'encoded_length='length(encoded) 'checksum='checksum
say 'PASS: Base64 Roundtrip v2 ooRexx port'
exit 0

base64Encode: procedure
  parse arg data
  alphabet='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
  result=''
  offset=1
  do while offset<=length(data)
    remaining=length(data)-offset+1
    byte1=c2d(substr(data,offset,1))
    byte2=0
    byte3=0
    if remaining>1 then byte2=c2d(substr(data,offset+1,1))
    if remaining>2 then byte3=c2d(substr(data,offset+2,1))
    digit1=byte1%4
    digit2=(byte1//4)*16+byte2%16
    digit3=(byte2//16)*4+byte3%64
    digit4=byte3//64
    result=result || substr(alphabet,digit1+1,1) || substr(alphabet,digit2+1,1)
    if remaining>1 then result=result || substr(alphabet,digit3+1,1)
    else result=result || '='
    if remaining>2 then result=result || substr(alphabet,digit4+1,1)
    else result=result || '='
    offset=offset+3
  end
  return result

base64Decode: procedure
  parse arg encoded
  result=''
  offset=1
  do while offset<=length(encoded)
    byte1=c2d(substr(encoded,offset,1))
    byte2=c2d(substr(encoded,offset+1,1))
    byte3=c2d(substr(encoded,offset+2,1))
    byte4=c2d(substr(encoded,offset+3,1))
    digit1=base64Digit(byte1)
    digit2=base64Digit(byte2)
    digit3=0
    digit4=0
    if byte3<>61 then digit3=base64Digit(byte3)
    if byte4<>61 then digit4=base64Digit(byte4)
    if digit1<0 | digit2<0 | digit3<0 | digit4<0 then call fail 'invalid Base64 digit'
    result=result || d2c(digit1*4+digit2%16)
    if byte3<>61 then result=result || d2c((digit2//16)*16+digit3%4)
    if byte4<>61 then result=result || d2c((digit3//4)*64+digit4)
    offset=offset+4
  end
  return result

base64Digit: procedure
  parse arg byte
  if byte>=65 & byte<=90 then return byte-65
  if byte>=97 & byte<=122 then return byte-97+26
  if byte>=48 & byte<=57 then return byte-48+52
  if byte=43 then return 62
  if byte=47 then return 63
  return -1

byteChecksum: procedure
  parse arg data
  checksum=0
  do index=1 to length(data)
    checksum=checksum+c2d(substr(data,index,1))
  end
  return checksum

fail: procedure
  parse arg message
  say 'FAIL:' message
  exit 1
