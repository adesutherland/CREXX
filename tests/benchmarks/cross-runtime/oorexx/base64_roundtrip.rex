/* Deterministic RFC 4648 Base64 encode/decode workload for ooRexx. */
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

say 'benchmark=base64_roundtrip repetitions='repetitions 'encoded_length='length(encoded) 'checksum='checksum
say 'PASS: Base64 Roundtrip ooRexx port'
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
  alphabet='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
  result=''
  offset=1
  do while offset<=length(encoded)
    digit1=pos(substr(encoded,offset,1),alphabet)-1
    digit2=pos(substr(encoded,offset+1,1),alphabet)-1
    char3=substr(encoded,offset+2,1)
    char4=substr(encoded,offset+3,1)
    digit3=0
    digit4=0
    if char3<>'=' then digit3=pos(char3,alphabet)-1
    if char4<>'=' then digit4=pos(char4,alphabet)-1
    result=result || d2c(digit1*4+digit2%16)
    if char3<>'=' then result=result || d2c((digit2//16)*16+digit3%4)
    if char4<>'=' then result=result || d2c((digit3//4)*64+digit4)
    offset=offset+4
  end
  return result

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
