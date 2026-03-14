options levelb
import rxfnsb

main: procedure
  myArray=.string[]
  add myArray 'Line 1'
  add myArray 'Line 2'
  add myArray 'Line 3.1'
  add myArray substr(myArray[3],1,length(myArray[3])-2)

  do i=1 to myArray[0]
     say i myArray[i]
  end