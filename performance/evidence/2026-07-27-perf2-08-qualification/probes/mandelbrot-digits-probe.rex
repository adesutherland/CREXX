/* PERF2-08 ordinary ooRexx decimal-mode qualification probe. */
parse arg size digits_value
if size='' then size=500
if digits_value='' then digits_value=20
numeric digits digits_value

result=mandelbrot(size)
say 'probe size='size 'digits='digits_value 'result='result
exit 0

mandelbrot: procedure
  parse arg size
  checksum=0
  byte_acc=0
  bit_num=0
  do y=0 to size-1
    ci=(2.0*y/size)-1.0
    do x=0 to size-1
      zrzr=0.0
      zi=0.0
      zizi=0.0
      cr=(2.0*x/size)-1.5
      escape=0
      do z=0 to 49 while escape=0
        zr=zrzr-zizi+cr
        zi=2.0*zr*zi+ci
        zrzr=zr*zr
        zizi=zi*zi
        if zrzr+zizi>4.0 then escape=1
      end

      byte_acc=byte_acc*2+escape
      bit_num=bit_num+1
      if bit_num=8 then do
        checksum=xor_byte(checksum,byte_acc)
        byte_acc=0
        bit_num=0
      end
      else if x=size-1 then do
        do pad=bit_num+1 to 8
          byte_acc=byte_acc*2
        end
        checksum=xor_byte(checksum,byte_acc)
        byte_acc=0
        bit_num=0
      end
    end
  end
  return checksum

xor_byte: procedure
  parse arg left,right
  result=0
  place=1
  do bit=1 to 8
    if left//2<>right//2 then result=result+place
    left=left%2
    right=right%2
    place=place*2
  end
  return result
