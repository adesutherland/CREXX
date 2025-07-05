
:: maximum 5 iterations over flags
for /l %%i in (0,1,5) do (
    set "char=!flags:~%%i,1!"
    if "!char!"=="P" (
        set "PRECOMP=P"
    )
    if "!char!"=="C" (
        set "COMPILE=C"
    )
   if "!char!"=="A" (
       set "ASM=A"
   )
   if "!char!"=="R" (
       set "RUN=R"
   )
)
set conf="L"
rem echo RUN=%run%
rem echo PRECOMP=%precomp%
rem echo COMPILE=%compile%
rem echo ASM=%asm%