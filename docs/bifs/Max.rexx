 /************************************************************************
 *.  M A X  9.4.3
 ************************************************************************/
 if !Bif_Arg.0 <1 then
   call Raise 40.3, 1
    call CheckArgs 'rNUM'||copies(' rNUM', !Bif_Arg.0 - 1)

    Max = !Bif_Arg.1
    do i = 2 to !Bif_Arg.0 by 1
      Next = !Bif_Arg.i
      if Max < Next then Max = Next
      end i
    return Max


